#!/usr/bin/tcc -run -I. -L. -lrandom -lcgic

#include <stdio.h>
#include <math.h>

#include "random.h"
#include "cgic.h"

void cgiInclude (FILE *fout, char *fn) {
	FILE *fin = fopen (fn, "r");
	char buf[256];
	while (fgets (buf, sizeof (buf), fin)) {
		fprintf (fout, "%s", buf);
	}
	fclose (fin);
}

int cgiMain () {

	char username[32];
	char password[256];

	srand (time (NULL));

	cgiHeaderContentType ("text/html");
	cgiInclude (cgiOut, "login.html");

	cgiFormString("username",username,sizeof(username));
	cgiFormString("password",password,sizeof(password));

	fprintf(cgiOut,"username: %s<br>",username);
	fprintf(cgiOut,"password: %s<br>",password);

	return 0;
}
