libsqlite3.a: sqlite3.o
	ar rcs libsqlite3.a sqlite3.o

sqlite3.o: sqlite3.c
	gcc -c sqlite3.c

clean:
	rm sqlite3.o libsqlite3.a

