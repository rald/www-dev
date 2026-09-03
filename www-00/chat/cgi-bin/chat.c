#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

#define DB_FILE "/home/fria/projects/chat/chat.db"
#define MAX_LINE 1024 

// Helper function to initialize database and table if not exists
int init_db(sqlite3 **db) {
    int rc = sqlite3_open(DB_FILE, db);
    if (rc != SQLITE_OK) {
        return rc;
    }
    char *err_msg = NULL;
    const char *sql = "CREATE TABLE IF NOT EXISTS messages ("
                      "timestamp TEXT, "
                      "payload TEXT);";
    rc = sqlite3_exec(*db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        return rc;
    }
    return SQLITE_OK;
}

void handle_get() {
    printf("Content-Type: text/plain\r\n\r\n");
    
    sqlite3 *db;
    if (init_db(&db) != SQLITE_OK) {
        sqlite3_close(db);
        return;
    }

    const char *sql = "SELECT timestamp, payload FROM messages ORDER BY rowid ASC;";
    sqlite3_stmt *res;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(res) == SQLITE_ROW) {
            const unsigned char *timestamp = sqlite3_column_text(res, 0);
            const unsigned char *payload = sqlite3_column_text(res, 1);
            printf("%s %s\n", timestamp, payload);
        }
    }
    
    sqlite3_finalize(res);
    sqlite3_close(db);
}

void handle_post() {
    char *len_str = getenv("CONTENT_LENGTH");
    if (!len_str) {
        printf("Status: 400 Bad Request\r\n\r\n");
        return;
    }

    int len = atoi(len_str);
    if (len > 1000) {
        printf("Status: 413 Payload Too Large\r\n\r\n");
        return;
    }

    char *post_data = malloc(len + 1);
    fread(post_data, 1, len, stdin);
    post_data[len] = '\0';

    sqlite3 *db;
    if (init_db(&db) != SQLITE_OK) {
        free(post_data);
        sqlite3_close(db);
        printf("Status: 500 Internal Server Error\r\n\r\n");
        return;
    }

    // 1. Generate timestamp string
    time_t rawtime;
    struct tm *timeinfo;
    char time_str[24]; 
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "[%Y-%m-%d %H:%M:%S]", timeinfo);

    // 2. Parse, truncate, and HTML escape payload into a temporary buffer
    char out_buf[MAX_LINE * 2] = {0};
    int out_idx = 0;
    
    char *split = strstr(post_data, ": ");
    int nick_len = split ? (int)(split - post_data) : len;
    
    int bytes_written = 0;
    int message_started = 0;
    int msg_chars_written = 0;

    for (int i = 0; i < len; i++) {
        if (!message_started && bytes_written >= 32 && post_data[i] != ':') {
            if (split && i < (split - post_data)) {
                continue; 
            }
        }
        if (post_data[i] == ':' && i == nick_len) {
            message_started = 1;
        }
        if (!message_started) {
            bytes_written++;
        }
        if (message_started && post_data[i] != ':' && post_data[i] != ' ') {
            if (msg_chars_written >= 512) {
                continue;
            }
            msg_chars_written++;
        }

        // Write escaped characters to out_buf safely
        switch (post_data[i]) {
            case '<':  out_idx += sprintf(&out_buf[out_idx], "&lt;");   break;
            case '>':  out_idx += sprintf(&out_buf[out_idx], "&gt;");   break;
            case '&':  out_idx += sprintf(&out_buf[out_idx], "&amp;");  break;
            case '"':  out_idx += sprintf(&out_buf[out_idx], "&quot;"); break;
            case '\'': out_idx += sprintf(&out_buf[out_idx], "&#x27;"); break;
            default:   out_buf[out_idx++] = post_data[i];
        }
    }
    out_buf[out_idx] = '\0';

    // 3. Prepare and execute the SQL Insert Statement safely
    const char *sql = "INSERT INTO messages (timestamp, payload) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, time_str, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, out_buf, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(post_data);

    printf("Status: 200 OK\r\nContent-Type: text/plain\r\n\r\nSent");
}

int main() {
    char *method = getenv("REQUEST_METHOD");
    if (!method) return 1;

    if (strcmp(method, "GET") == 0) {
        handle_get();
    } else if (strcmp(method, "POST") == 0) {
        handle_post();
    }
    return 0;
}
