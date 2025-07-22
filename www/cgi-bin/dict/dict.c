#include "cgic.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int cgiMain() {

	char word[256];

	cgiHeaderContentType("text/html");

	fprintf(cgiOut,
		"<html>"
		"<head>"
		"<meta name='viewport' content='width=device-width, initial-scale=1.0' />"
		"</head>"
		"<body>"
	);

	cgiFormStringNoNewlines("word",word,256);

	fprintf(cgiOut,
		"<form action='dict.cgi' method='post'>"
		"<input name='word' type='text' value='%s'>&nbsp;"
		"<input name='search' type='submit' value='search'>"
		"</form>"
		,word
	);

	if(cgiFormSubmitClicked("search")==cgiFormSuccess) {

		FILE *fin=fopen("../../../csw/dict.txt","r");

		char w[256];
		char d[1024];

		bool found=false;

		while((fscanf(fin,"%[^\t]\t%[^\n]\n",w,d)==2)) {
			if(!strcasecmp(word,w)) {
				fprintf(cgiOut,"%s",d);
				found=true;
				break;
			}
		}

		if(!found) {
			fprintf(cgiOut,"'%s' not found.",word);
		}

	}

	fprintf(cgiOut,
		"</body>"
		"</html>"
	);

	return 0;
}