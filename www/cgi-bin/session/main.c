#include <stdio.h>
#include <sqlite3.h>
#include "session.h"
#include "cgic.h"

int cgiMain(void) {
    sqlite3 *db;
    if (sqlite3_open("sessions.db", &db) != SQLITE_OK) {
        cgiHeaderContentType("text/plain\n\n");
        fprintf(cgiOut, "Cannot open DB\n");
        return 1;
    }

    Session *sess = session_start(db);
    if (!sess) {
        cgiHeaderContentType("text/plain\n\n");
        fprintf(cgiOut, "Cannot start session\n");
        sqlite3_close(db);
        return 1;
    }

    cgiHeaderContentType("text/html\n\n");

    char username[100] = "";
    if (cgiFormString("username", username, sizeof(username)) == cgiFormSuccess) {
        session_set(sess, "username", username);
        session_commit(sess);
    }

    char current_user[100] = "";
    if (session_get(sess, "username", current_user, sizeof(current_user)) == 0) {
        fprintf(cgiOut, "<p>Welcome back, %s!</p>\n", current_user);
    } else {
        fprintf(cgiOut, "<p>Hello new visitor!</p>\n");
    }

    fprintf(cgiOut,
            "<form method='POST'>"
            "Username: <input type='text' name='username' value='%s'/>"
            "<input name='start' type='submit' value='Start'/>"
            "<input name='end' type='submit' value='End'/>"
            "</form>\n",
            current_user);

    // Example to end session if you want (uncomment to use)

    if (cgiFormSubmitClicked("end") == cgiFormSuccess) {
        session_end(sess);
        // Do not use sess pointer after this
    } else {
        free(sess); // free allocated session structure
    }

    sqlite3_close(db);
    return 0;
}
