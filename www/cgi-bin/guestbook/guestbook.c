#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

#include <cgic.h>

#define DB_PATH    "guestbook.db"
#define PAGE_SIZE  5

// Ensure the database and table exist
void ensureDatabase () {
  sqlite3 *db;
  sqlite3_open (DB_PATH, &db);
  sqlite3_exec (db,
                "CREATE TABLE IF NOT EXISTS entries (id INTEGER PRIMARY KEY AUTOINCREMENT, name VARCHAR(32), message VARCHAR(1024))",
                NULL, NULL, NULL);
  sqlite3_close (db);
}

// Escape HTML special chars
void htmlEscape (const char *src, char *dest, size_t maxlen) {
  size_t len = 0;
  for (; *src && len + 8 < maxlen; ++src) {
    if (*src == '<') {
      strcat (dest, "&lt;");
      len += 4;
    } else if (*src == '>') {
      strcat (dest, "&gt;");
      len += 4;
    } else if (*src == '&') {
      strcat (dest, "&amp;");
      len += 5;
    } else if (*src == '"') {
      strcat (dest, "&quot;");
      len += 6;
    } else {
      dest[len++] = *src;
      dest[len] = 0;
    }
  }
  dest[len] = 0;
}

int cgiMain () {
  cgiHeaderContentType ("text/html");
  ensureDatabase ();

  char name[33] = "", message[1025] = "";
  cgiFormStringNoNewlines ("name", name, sizeof (name));
  cgiFormStringNoNewlines ("message", message, sizeof (message));
  int page = 1;
  cgiFormInteger ("page", &page, 1);
  if (page < 1)
    page = 1;

  // Handle new message
  if (strlen (message) > 0) {
    sqlite3 *db;
    sqlite3_open (DB_PATH, &db);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2 (db,
                        "INSERT INTO entries (name, message) VALUES (?, ?)",
                        -1, &stmt, 0);
    sqlite3_bind_text (stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text (stmt, 2, message, -1, SQLITE_STATIC);
    sqlite3_step (stmt);
    sqlite3_finalize (stmt);
    sqlite3_close (db);
  }

  fprintf (cgiOut,
           "<!DOCTYPE html>"
           "<html>"
           "<head>"
           "<meta name='viewport' content='width=device-width, initial-scale=1.0' />"
           "<title>Guestbook</title>"
           "<link rel='stylesheet' href='../../style.css'>"
           "</head>"
           "<body>"
           "<div class='container'>"
           "<header>"
           "<h1>Guestbook</h1>"
           "</header>"
           "<main>"
           "<section class='post'>"
           "<form method='POST' action='guestbook.cgi'>"
           "<table width='100%%'>"
           "<tr><td>Name:</td><td><input style='width:100%%' type='text' name='name' maxlength='32' required /></td></tr>"
           "<tr><td>Message:</td><td><textarea style='width:100%%;height:128px;resize:none;' name='message' maxlength='1024' required></textarea></td></tr>"
           "<tr><td align='right' colspan='2'>"
           "<input type='reset' value='Clear'/>&nbsp;"
           "<input type='submit' value='Sign'/>"
           "</form></td></tr>"
           "</table>" "</form>" "</section>");


  // Count total entries
  sqlite3 *db;
  sqlite3_open (DB_PATH, &db);
  int total = 0;
  sqlite3_stmt *countstmt;
  sqlite3_prepare_v2 (db, "SELECT COUNT(*) FROM entries", -1, &countstmt, 0);
  if (sqlite3_step (countstmt) == SQLITE_ROW)
    total = sqlite3_column_int (countstmt, 0);
  sqlite3_finalize (countstmt);

  // Fetch paginated entries
  int offset = (page - 1) * PAGE_SIZE;
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2 (db,
                      "SELECT name, message,strftime('%Y-%m-%d %l:%M:%S %p',datetime,'+8 hours') as datetime FROM entries ORDER BY id DESC LIMIT ? OFFSET ?",
                      -1, &stmt, 0);
  sqlite3_bind_int (stmt, 1, PAGE_SIZE);
  sqlite3_bind_int (stmt, 2, offset);
  fprintf (cgiOut, "<h2 align='center'>Page %d</h2>", page);

  while (sqlite3_step (stmt) == SQLITE_ROW) {
    const unsigned char *name_raw = sqlite3_column_text (stmt, 0);
    const unsigned char *msg_raw = sqlite3_column_text (stmt, 1);
    const unsigned char *datetime_raw = sqlite3_column_text (stmt, 2);
    char name_html[256] = "", msg_html[1024] = "", datetime_html[256];
    if (name_raw)
      htmlEscape ((const char *) name_raw, name_html, sizeof (name_html));
    if (msg_raw)
      htmlEscape ((const char *) msg_raw, msg_html, sizeof (msg_html));
    if (datetime_raw)
      htmlEscape ((const char *) datetime_raw, datetime_html,
                  sizeof (datetime_html));


    fprintf (cgiOut, "<section class='post'>");
    fprintf (cgiOut, "<div class='datetime'>%s</div>", datetime_html);
    fprintf (cgiOut, "<div class='clear'></div>");
    fprintf (cgiOut, "<div class='name'>%s</div>", name_html);
    fprintf (cgiOut, "<hr>");
    fprintf (cgiOut, "<p class='content'>%s</p>", msg_html);
    fprintf (cgiOut, "</section>");

  }
  sqlite3_finalize (stmt);

  fprintf (cgiOut, "<center>");

  // Pagination links
  int page_count = (total + PAGE_SIZE - 1) / PAGE_SIZE;
  fprintf (cgiOut, "<p>");
  if (page > 1)
    fprintf (cgiOut, "<a href='guestbook.cgi?page=%d'>&lt;Prev</a> ",
             page - 1);
  for (int i = 1; i <= page_count; ++i) {
    if (i == page)
      fprintf (cgiOut, " <b>%d</b> ", i);
    else
      fprintf (cgiOut, " <a href='guestbook.cgi?page=%d'>%d</a> ", i, i);
  }
  if (page < page_count)
    fprintf (cgiOut, " <a href='guestbook.cgi?page=%d'>Next&gt;</a>",
             page + 1);
  fprintf (cgiOut, "</p>");

  fprintf (cgiOut, "</center>");


  fprintf (cgiOut, "</main>");


  fprintf (cgiOut, "<footer>");
  fprintf (cgiOut, "<h4>Since 2022-09-01 by FriaElAgua</h4>");
  fprintf (cgiOut, "</footer>");

  fprintf (cgiOut, "</div>");

  fprintf (cgiOut, "</body>");
  fprintf (cgiOut, "</html>");


  sqlite3_close (db);
  return 0;
}
