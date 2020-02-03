#ifndef __RBTREE_MAP_TEST_H__
#define __RBTREE_MAP_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/core/RBTree_Map.h>

typedef struct rbtree_map_test_s RBTree_Map_Test;

struct rbtree_map_test_s{
	Test parent;

	int (*construct)(Test *,char *init_str);
	int (*deconstruct)(Test *);
	int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
	int (*setup)(Test *);
    void *(*teardown)(Test *);
    int (*test_add)(RBTree_Map_Test *test);
    int (*test_count)(RBTree_Map_Test *test);
    int (*test_clear)(RBTree_Map_Test *test);
    int (*test_clear_string_value)(RBTree_Map_Test *test);
    int (*test_clear_object_value)(RBTree_Map_Test *test);
    int (*test_remove)(RBTree_Map_Test *test);
    int (*test_search_string_key)(RBTree_Map_Test *test);
    int (*test_search_all_string_key)(RBTree_Map_Test *test);
    int (*test_search_int_key)(RBTree_Map_Test *test);
    int (*test_search_all_int_key)(RBTree_Map_Test *test);

    Map *map;
};

#endif
