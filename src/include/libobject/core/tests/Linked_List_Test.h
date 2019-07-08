#ifndef __MODULE_TEST_H__
#define __MODULE_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/core/Linked_List.h>

typedef struct linked_list_test_s Linked_List_Test;

struct linked_list_test_s{
	Test parent;

	int (*construct)(Test *,char *init_str);
	int (*deconstruct)(Test *);
	int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
	int (*setup)(Test *);
    void *(*teardown)(Test *);
    int (*test_add)(Linked_List_Test *test);
    int (*test_count)(Linked_List_Test *test);
    int (*test_remove)(Linked_List_Test *test);
    int (*test_clear)(Linked_List_Test *test);
    int (*test_clear_string_value)(Linked_List_Test *test);

    Linked_List *list;
};

#endif
