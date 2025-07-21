#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>


#include "cgic.h"
#include "session.h"



/**
 * Reads the entire contents of a text file into a dynamically allocated buffer.
 * - The buffer is NUL-terminated.
 * - Returns NULL on any error (open, allocation, I/O, or overflow), and sets errno.
 * - The caller must free() the returned buffer.
 */
char* readAll(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    // Detect file size
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long filesize_long = ftell(f);
    if (filesize_long < 0) { fclose(f); return NULL; }
    size_t filesize = (size_t)filesize_long;
    rewind(f);

    // Prevent size_t overflow in allocation
    if (filesize > SIZE_MAX - 1) { fclose(f); errno = EOVERFLOW; return NULL; }
    char* buffer = malloc(filesize + 1);
    if (!buffer) { fclose(f); return NULL; }

    size_t readsize = fread(buffer, 1, filesize, f);
    if (readsize != filesize) {
        // Possible I/O error or file changed during read
        free(buffer); fclose(f); return NULL;
    }
    buffer[readsize] = '\0'; // Always NUL-terminate

    fclose(f);
    return buffer;
}

void cgiInclude(FILE *fout,char *filename) {
	int ch;
	FILE *fin=fopen(filename,"r");
	while((ch=fgetc(fin))!=EOF) {
		fputc(ch,fout);
	}
	fclose(fin);
}


int cgiMain() {
	sqlite3 *db;
  sqlite3_stmt *stmt;

	char username[33]="";
	char password[257]="";

	cgiHeaderContentType("text/html");

	cgiInclude(cgiOut,"login.html");

	if(	cgiFormString("username",username,sizeof(username))!=cgiFormSuccess ||
			cgiFormString("password",password,sizeof(password))!=cgiFormSuccess
	) {
		fprintf(cgiOut, "Username and Password required\n");
		return 1;
	}

	if (sqlite3_open("login.db", &db) != SQLITE_OK) {
		fprintf(cgiOut, "Cannot open login.db\n");
		return 1;
	}

	char *sql="SELECT username,password FROM users WHERE username=? AND password=?";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(cgiOut, "Error sqlite3_prepare_v2\n");
		return 1;
	}

	sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

	sqlite3_bind_text(stmt, 2, password, -1, SQLITE_TRANSIENT);

	if (sqlite3_step(stmt) != SQLITE_ROW) {
		fprintf(cgiOut, "Invalid Username or Password\n");
	  sqlite3_finalize(stmt);
 		sqlite3_close(db);
		return 1;
  }

  sqlite3_finalize(stmt);
 	sqlite3_close(db);

	if (sqlite3_open("sessions.db", &db) != SQLITE_OK) {
		fprintf(cgiOut, "Cannot open sessions.db\n");
		return 1;
  }

	Session *session = session_start(db);

	if (!session) {
		fprintf(cgiOut, "Cannot start session\n");
		sqlite3_close(db);
		return 1;
	}

  session_set(session,"username",username);
  session_commit(session);

	return 0;
}
