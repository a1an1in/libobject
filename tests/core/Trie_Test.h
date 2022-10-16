#ifndef __TRIE_TEST_H__
#define __TRIE_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/core/Trie.h>

typedef struct trie_test_s Trie_Test;

struct trie_test_s{
	Test parent;

	int (*construct)(Test *,char *init_str);
	int (*deconstruct)(Test *);
	int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
	int (*setup)(Test *);
    int (*teardown)(Test *);
    int (*test_insert)(Trie_Test *);
    int (*test_search)(Trie_Test *);
    int (*test_search_prefix)(Trie_Test *);
    int (*test_delete)(Trie_Test *);

    Trie *trie;
};

#endif
