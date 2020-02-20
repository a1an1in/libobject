#ifndef __TRIE_H__
#define __TRIE_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Trie_s Trie;

typedef struct trie_node_s {
    int end;
    int pass;
    struct trie_node_s **table;
} trie_node_t;

struct Trie_s{
	Obj parent;

	int (*construct)(Trie *,char *);
	int (*deconstruct)(Trie *);

	/*virtual methods reimplement*/
	int (*set)(Trie *trie, char *attrib, void *value);
    void *(*get)(Trie *, char *attrib);
    int (*insert)(Trie *trie, char *word);
    int (*search)(Trie *trie, char *word);
    int (*search_prefix)(Trie *trie, char *word);
    int (*delete)(Trie *trie, char *word);
    int (*set_char_set)(Trie *trie, char *word);

    struct trie_node_s *root;
    int node_count;
    char *char_set;
};

#endif
