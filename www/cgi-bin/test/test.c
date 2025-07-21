#!/usr/bin/tcc -run random.c -I. -L. -I /data/data/com.termux/files/home/projects/cgic -L /data/data/com.termux/files/home/projects/cgic -lcgic

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

	srand (time (NULL));

	cgiHeaderContentType ("text/html");
	cgiInclude (cgiOut, "login.html");

	return 0;
}
