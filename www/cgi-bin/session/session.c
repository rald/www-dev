#include "session.h"

#include "cgic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SESSION_COOKIE_NAME "SESSIONID"
#define SESSION_EXPIRY_SECONDS 3600 // 1 hour
#define SESSION_DB_TABLE_CREATE \
  "CREATE TABLE IF NOT EXISTS sessions (" \
  "session_id TEXT PRIMARY KEY," \
  "data TEXT," \
  "expiry INTEGER);"

// Maximum buffer size for serialized session data
#define MAX_SESSION_DATA 2048

// For simplicity, store session data as key=value\n lines in a string
struct Session {
    sqlite3 *db;
    char session_id[65];
    char data[MAX_SESSION_DATA]; // serialized as key=value\n
    time_t expiry;
    int dirty; // whether data changed
};

// Helper: generate random hex string as session id (32 chars + nul)
static void generate_session_id(char *buf, size_t bufsize) {
    const char *hex = "0123456789abcdef";
    FILE *urandom = fopen("/dev/urandom", "rb");
    if (!urandom) {
        srand(time(NULL));
        for (size_t i = 0; i < bufsize - 1; i++) {
            buf[i] = hex[rand() % 16];
        }
        buf[bufsize - 1] = 0;
        return;
    }
    for (size_t i = 0; i < bufsize - 1; i++) {
        unsigned char c;
        fread(&c, 1, 1, urandom);
        buf[i] = hex[c % 16];
    }
    buf[bufsize - 1] = 0;
    fclose(urandom);
}

// Parse cookie string for session id (returns malloc'ed string or NULL)
static char *get_session_id_from_cookie() {
    char sid[65] = {0};
    if (cgiCookieString(SESSION_COOKIE_NAME, sid, sizeof(sid)) == cgiFormSuccess) {
        if (strlen(sid) > 0) {
            return strdup(sid);
        }
    }
    return NULL;
}

// Send Set-Cookie header with session id and expiry
static void send_set_cookie(const char *session_id) {
    time_t now = time(NULL) + SESSION_EXPIRY_SECONDS;
    struct tm *tm_exp = gmtime(&now);
    char expires_str[64];
    strftime(expires_str, sizeof(expires_str), "%a, %d %b %Y %H:%M:%S GMT", tm_exp);
    fprintf(cgiOut,
        "Set-Cookie: %s=%s; Path=/; Expires=%s; HttpOnly\n",
        SESSION_COOKIE_NAME,
        session_id,
        expires_str);
}

// Create sessions table if needed
static int create_sessions_table(sqlite3 *db) {
    char *err = NULL;
    int rc = sqlite3_exec(db, SESSION_DB_TABLE_CREATE, 0, 0, &err);
    if (rc != SQLITE_OK) {
        if (err) {
            fprintf(stderr, "Failed to create session table: %s\n", err);
            sqlite3_free(err);
        }
    }
    return rc;
}

// Deserialize key=value\n lines from session->data into simple key-value store (in-memory)
// For this example only store keys and values in separate arrays
// Here we store session data flat in session->data string, provide helper to get/set keys inside it

// Helper to get value of key from serialized data
static int session_data_get(Session *session, const char *key, char *out_val, int out_len) {
    if (!session || !key || !out_val) return -1;
    // Parse lines: key=value\n
    char *data = session->data;
    char *line = strtok(data, "\n");
    while (line) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = 0;
            if (strcmp(line, key) == 0) {
                strncpy(out_val, eq + 1, out_len);
                out_val[out_len - 1] = 0;
                // restore '='
                *eq = '=';
                return 0;
            }
            *eq = '=';
        }
        line = strtok(NULL, "\n");
    }
    return -1;
}

// Helper to set or replace key=value in session->data string
static void session_data_set(Session *session, const char *key, const char *value) {
    if (!session || !key || !value) return;

    // For simplicity, parse current data into lines, rebuild replacing key line, or add if missing
    char newdata[MAX_SESSION_DATA] = {0};
    char *dst = newdata;
    int replaced = 0;

    // Duplicate original data to work on without modifying original
    char data_copy[MAX_SESSION_DATA];
    strncpy(data_copy, session->data, MAX_SESSION_DATA-1);

    char *line = strtok(data_copy, "\n");
    while (line) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = 0;
            if (strcmp(line, key) == 0) {
                // Replace this line
                int n = snprintf(dst, sizeof(newdata) - (dst - newdata), "%s=%s\n", key, value);
                dst += n;
                replaced = 1;
            } else {
                // Copy unchanged line
                int n = snprintf(dst, sizeof(newdata) - (dst - newdata), "%s=%s\n", line, eq + 1);
                dst += n;
            }
            *eq = '=';
        }
        line = strtok(NULL, "\n");
    }
    // If key not replaced, append it
    if (!replaced) {
        int n = snprintf(dst, sizeof(newdata) - (dst - newdata), "%s=%s\n", key, value);
        dst += n;
    }
    // Copy back
    strncpy(session->data, newdata, MAX_SESSION_DATA-1);
    session->dirty = 1;
}

// Load session row from DB by session_id, returns 0 on OK, -1 if not found
static int session_load(Session *session) {
    const char *sql = "SELECT data, expiry FROM sessions WHERE session_id = ?1;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(session->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_text(stmt, 1, session->session_id, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // Get expiry timestamp
        sqlite3_int64 expiry = sqlite3_column_int64(stmt, 1);
        time_t now = time(NULL);
        if ((time_t)expiry < now) {
            sqlite3_finalize(stmt);
            return -1; // expired
        }
        const unsigned char *data_text = sqlite3_column_text(stmt, 0);
        if (data_text) {
            strncpy(session->data, (const char *)data_text, MAX_SESSION_DATA-1);
        } else {
            session->data[0] = 0;
        }
        session->expiry = expiry;
        sqlite3_finalize(stmt);
        session->dirty = 0;
        return 0;
    } else {
        sqlite3_finalize(stmt);
        return -1;
    }
}

// Write session to DB (insert or update)
static int session_save(Session *session) {
    const char *sql = "INSERT INTO sessions(session_id, data, expiry) "
                      "VALUES (?1, ?2, ?3) "
                      "ON CONFLICT(session_id) DO UPDATE SET data=excluded.data, expiry=excluded.expiry;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(session->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;

    time_t now = time(NULL);
    session->expiry = now + SESSION_EXPIRY_SECONDS;

    sqlite3_bind_text(stmt, 1, session->session_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, session->data, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, (sqlite3_int64)session->expiry);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc == SQLITE_DONE) {
        session->dirty = 0;
        return 0;
    }
    return rc;
}

// Public API functions ------------------------

// Initialize or resume session
Session* session_start(sqlite3 *db) {
    if (!db) return NULL;
    // Ensure DB table exists
    if (create_sessions_table(db) != SQLITE_OK) {
        return NULL;
    }

    Session *session = calloc(1, sizeof(Session));
    if (!session) return NULL;

    session->db = db;

    // Try get session id from cookie
    char *sid = get_session_id_from_cookie();
    if (sid) {
        strncpy(session->session_id, sid, sizeof(session->session_id)-1);
        free(sid);
        // Try load from DB
        if (session_load(session) == 0) {
            // Loaded session; update expiry on commit later
            return session;
        }
        // Session id invalid or expired, create new
    }

    // Create new session id
    generate_session_id(session->session_id, sizeof(session->session_id));
    session->data[0] = 0;
    session->expiry = time(NULL) + SESSION_EXPIRY_SECONDS;
    session->dirty = 1;

    // Send cookie header for new session
    send_set_cookie(session->session_id);

    return session;
}

int session_get(Session *session, const char *key, char *value, int val_len) {
    if (!session || !key || !value) return -1;
    // Copy data string because strtok modifies it
    char data_copy[MAX_SESSION_DATA];
    strncpy(data_copy, session->data, MAX_SESSION_DATA-1);
    data_copy[MAX_SESSION_DATA-1] = 0;

    char *line = strtok(data_copy, "\n");
    while (line) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = 0;
            if (strcmp(line, key) == 0) {
                strncpy(value, eq + 1, val_len-1);
                value[val_len-1] = 0;
                return 0;
            }
            *eq = '=';
        }
        line = strtok(NULL, "\n");
    }
    return -1;
}

void session_set(Session *session, const char *key, const char *value) {
    if (!session || !key || !value) return;

    // Use a temporary buffer and rebuild serialized data
    char newdata[MAX_SESSION_DATA] = {0};
    char *dst = newdata;
    int replaced = 0;

    char data_copy[MAX_SESSION_DATA];
    strncpy(data_copy, session->data, MAX_SESSION_DATA-1);

    char *line = strtok(data_copy, "\n");
    while (line) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = 0;
            if (strcmp(line, key) == 0) {
                int n = snprintf(dst, sizeof(newdata) - (dst - newdata), "%s=%s\n", key, value);
                dst += n;
                replaced = 1;
            } else {
                int n = snprintf(dst, sizeof(newdata) - (dst - newdata), "%s=%s\n", line, eq + 1);
                dst += n;
            }
            *eq = '=';
        }
        line = strtok(NULL, "\n");
    }
    if (!replaced) {
        int n = snprintf(dst, sizeof(newdata) - (dst - newdata), "%s=%s\n", key, value);
        dst += n;
    }

    strncpy(session->data, newdata, MAX_SESSION_DATA-1);
    session->data[MAX_SESSION_DATA-1] = 0;
    session->dirty = 1;
}

// Save session data to DB
int session_commit(Session *session) {
    if (!session || !session->dirty) return 0;
    return session_save(session);
}

// Delete session from DB and clear cookie
void session_end(Session *session) {
    if (!session) return;
    // Delete DB record
    const char *sql = "DELETE FROM sessions WHERE session_id = ?1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(session->db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, session->session_id, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    // Expire cookie by sending past date
    fprintf(cgiOut,
        "Set-Cookie: %s=deleted; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; HttpOnly\n",
        SESSION_COOKIE_NAME);
    free(session);
}
