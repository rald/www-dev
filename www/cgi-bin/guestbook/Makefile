guestbook.cgi: guestbook.c
	gcc guestbook.c -o guestbook.cgi -lcgic -lsqlite3 -ldl -lpthread -I ../../../cgic -L ../../../cgic -I ../../../sqlite3 -L ../../../sqlite3
	chmod 755 guestbook.cgi

clean:
	rm guestbook.cgi
