#ifndef TRIE_H
#define TRIE_H

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct Trie Trie;

struct Trie {
	bool mark;
	Trie *next[26];
};

Trie *Trie_New();
void Trie_AddWord(Trie *trie,char *word);
void Trie_Walk(Trie *trie,int depth);

#endif /* TRIE_H */
