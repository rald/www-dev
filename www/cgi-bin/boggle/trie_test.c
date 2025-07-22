#include "trie.h"

#include <stdio.h>

int main(void) {

	Trie *trie=Trie_New();
	char line[256];

	FILE *fin=fopen("../../../csw/csw.txt","r");
	while(fgets(line,256,fin)) {
		char *p=strchr(line,'\n');
		if(p) *p='\0';
		Trie_AddWord(trie,line);
	}
	fclose(fin);

	Trie_Walk(trie,-1);


	return 0;
}