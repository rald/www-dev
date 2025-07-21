email.cgi: email.c
	gcc email.c -o email.cgi -DSMTP_OPENSSL -I ../../../cgic -L ../../../cgic -I ../../../smtp-client -L ../../../smtp-client -lsmtp -lcgic -lssl -lcrypto
	chmod 755 email.cgi

clean:
	rm email.cgi
