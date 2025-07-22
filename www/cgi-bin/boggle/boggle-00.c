#include "cgic.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char *dice[]={
	"AAEEGN",
	"ABBJOO",
	"ACHOPS",
	"AFFKPS",
	"AOOTTW",
	"CIMOTU",
	"DEILRX",
	"DELRVY",
	"DISTTY",
	"EEGHNW",
	"EEINSU",
	"EHRTVW",
	"EIOSST",
	"ELRTTY",
	"HIMNUQ",
	"HLNNRZ",
};

char board[17];

void shuffle(char *dice[]) {
	for(int i=15;i>0;i--) {
		int j=rand()%(i+1);
		char *tmp=dice[i];
		dice[i]=dice[j];
		dice[j]=tmp;
	}
}

void initBoard(char board[]) {
	int k=0;
	for(int j=0;j<4;j++) {
		for(int i=0;i<4;i++) {
			board[k]=dice[k][rand()%6];
			k++;
		}
	}
}

void printBoard(char board[]) {
	fprintf(cgiOut,"\t\t<table border cellspacing='1' cellpadding='10' style='font-family:monospace;font-size:12pt;'>\n");
	int k=0;
	for(int j=0;j<4;j++) {
		fprintf(cgiOut,"\t\t\t<tr>\n");
		for(int i=0;i<4;i++) {
			fprintf(cgiOut,"\t\t\t\t<th>%c</th>\n",board[k++]);
		}
		fprintf(cgiOut,"\t\t\t</tr>\n");
	}
	fprintf(cgiOut,"\t\t</table><br>\n");
}


int cgiMain() {

	srand(time(NULL));

	cgiHeaderContentType("text/html");

	fprintf(cgiOut,
		"<html>\n"
		"\t<head>\n"
		"\t\t<meta name='viewport' content='width=device-width, initial-scale=1.0' />\n"
		"\t</head>\n"
		"\t<body>\n"
	);

	cgiFormString("board",board,sizeof(board));

	if(strlen(board)!=16) {
		shuffle(dice);
		initBoard(board);
	}

	printBoard(board);

	fprintf(cgiOut,
		"\t\t<form action='boggle.cgi' method='post'>\n"
		"\t\t\t<input type='hidden' name='board' value='%16s'>\n"
		"\t\t\t<input name='guess' type='text'>&nbsp;\n"
		"\t\t\t<input name='send' type='submit' value='send'><br>\n"
		"\t\t</form>\n",
		board
	);

	fprintf(cgiOut,"\t</body>\n");
	fprintf(cgiOut,"</head>\n");

	return 0;
}
