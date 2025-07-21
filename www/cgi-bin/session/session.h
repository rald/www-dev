#ifndef SESSION_H
#define SESSION_H

#include <sqlite3.h>

// Opaque session object
typedef struct Session Session;

// Start or resume a session.
// On success returns pointer to Session, NULL on failure.
// It outputs Set-Cookie header only if a new session is created.
Session* session_start(sqlite3 *db);

// Get value by key from session; returns 0 if found, else nonzero
int session_get(Session *session, const char *key, char *value, int val_len);

// Set key-value pair in session (in-memory)
void session_set(Session *session, const char *key, const char *value);

// Save session data to DB and update expiry
int session_commit(Session *session);

// Ends session by deleting from DB, unset cookie, and frees resources
void session_end(Session *session);

#endif
