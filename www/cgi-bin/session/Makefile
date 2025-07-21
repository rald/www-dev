all: session.cgi login.cgi

session.cgi: main.c
	gcc main.c -o session.cgi -lsqlite3 -lcgic -lsession -I. -L. -I ../../../cgic -L ../../../cgic
	chmod 755 session.cgi

login.cgi: login.c
	gcc login.c -o login.cgi -lsqlite3 -lcgic -lsession -I. -L. -I ../../../cgic -L ../../cgic
	chmod 755 login.cgi



clean:
	rm login.cgi session.cgi
