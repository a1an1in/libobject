#ifndef __VECTOR_TEST_H__
#define __VECTOR_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/core/Vector.h>

typedef struct vector_test_s Vector_Test;

struct vector_test_s{
	Test parent;

	int (*construct)(Test *,char *init_str);
	int (*deconstruct)(Test *);
	int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
	int (*setup)(Test *);
    void *(*teardown)(Test *);
    int (*test_int_vector_add)(Test *test);
    int (*test_int_vector_remove)(Test *test);
    int (*test_int_vector_count)(Test *test);
    int (*test_int_vector_clear)(Test *test);
    int (*test_int_vector_to_json)(Test *test);
    int (*test_string_vector_to_json)(Test *test);
    int (*test_obj_vector_to_json)(Test *test);
    int (*test_int_vector_set_init_data)(Test *test);
    int (*test_string_vector_set_init_data)(Test *test);
    int (*test_obj_vector_set_init_data)(Test *test);

    Vector *vector;
};

#endif
