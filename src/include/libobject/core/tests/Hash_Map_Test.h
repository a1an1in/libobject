#ifndef __HASH_MAP_TEST_H__
#define __HASH_MAP_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/core/Hash_Map.h>

typedef struct hash_map_test_s Hash_Map_Test;

struct hash_map_test_s{
	Test parent;

	int (*construct)(Test *,char *init_str);
	int (*deconstruct)(Test *);
	int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
	int (*setup)(Test *);
    void *(*teardown)(Test *);
    int (*test_add)(Hash_Map_Test *test);
    int (*test_count)(Hash_Map_Test *test);
    int (*test_clear)(Hash_Map_Test *test);
    int (*test_clear_string_value)(Hash_Map_Test *test);
    int (*test_clear_object_value)(Hash_Map_Test *test);
    int (*test_remove)(Hash_Map_Test *test);
    int (*test_search_string_key)(Hash_Map_Test *test);
    int (*test_search_all_string_key)(Hash_Map_Test *test);
    int (*test_search_int_key)(Hash_Map_Test *test);
    int (*test_search_all_int_key)(Hash_Map_Test *test);

    Hash_Map *map;
};

#endif
