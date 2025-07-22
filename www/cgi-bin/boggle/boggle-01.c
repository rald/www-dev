#include "cgic.h"
#include "trie.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
bool graph[16];
char **words=NULL;
int nwords=0;

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


void dfs(Trie *trie,int x,int y,int d) {
	static char word[17];

	if(x<0 || x>3 || y<0 || y>3) return;

	int k=y*4+x;

	if(graph[k]) return;

	trie=trie->next[board[k]-'A'];

	if(trie==NULL) return;

	word[d++]=board[k];

	if(trie->mark && d>=4) {
		word[d]='\0';

		bool found=false;
		for(int i=0;i<nwords;i++) {
			if(!strcasecmp(word,words[i])) {
				found=true;
				break;
			}
		}

		if(!found) {
			words=realloc(words,sizeof(*words)*(nwords+1));
			words[nwords++]=strdup(word);

			fprintf(cgiOut,"%s ",word);
		}
	}

	graph[k]=true;

	for(int j=-1;j<=1;j++)
		for(int i=-1;i<=1;i++)
	  	if(i || j)
				dfs(trie,x+i,y+j,d);

	graph[k]=false;
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
		"\t\t</form><br>\n\n",
		board
	);



	Trie *trie=Trie_New();
	char line[256];
	FILE *fin=fopen("csw.txt","r");
	while(fgets(line,256,fin)) {
		char *p=strchr(line,'\n');
		if(p) *p='\0';
		Trie_AddWord(trie,line);
	}
	fclose(fin);

	for(int i=0;i<16;i++)
		graph[i]=false;

	for(int j=0;j<4;j++)
		for(int i=0;i<4;i++)
			dfs(trie,i,j,0);

	fprintf(cgiOut,"\n\n\t\t<br>\n");

	fprintf(cgiOut,"\t</body>\n");
	fprintf(cgiOut,"</head>\n");

	return 0;
}
