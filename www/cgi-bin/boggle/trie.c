#include "trie.h"

Trie *Trie_New() {
	Trie *trie=malloc(sizeof(*trie));
	trie->mark=false;
	for(int i=0;i<26;i++) {
		trie->next[i]=NULL;
	}
	return trie;
}

void Trie_AddWord(Trie *trie,char *word) {
	if(word==NULL || *word=='\0') return;
	Trie *curr=trie;
	char *let=word;
	while(*let) {
		int nxtlet=toupper(*let)-'A';
		if(nxtlet<0 || nxtlet>25) return;
		if(curr->next[nxtlet]==NULL) {
			curr->next[nxtlet]=Trie_New();
		}
		curr=curr->next[nxtlet];
		let++;
	}
	curr->mark=true;
}

void Trie_Walk(Trie *trie,int depth) {
	static char buf[17];
	depth++;
	if(trie->mark) {
		buf[depth]='\0';
		puts(buf);
	}
	for(int i=0;i<26;i++) {
		if(trie->next[i]!=NULL) {
			buf[depth]=i+'A';
			Trie_Walk(trie->next[i],depth);
		}
	}
}
