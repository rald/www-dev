test.cgi: test.c test.html
	gcc test.c -o test.cgi -I ../../../cgic -L ../../../cgic -I ../../../libctemplate -L ../../../libctemplate -lcgic -lctemplate
	chmod 755 test.cgi

clean:
	rm test.cgi
