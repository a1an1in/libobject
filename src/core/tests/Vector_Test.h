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
    int *(*teardown)(Test *);
    int (*test_vector_add)(Test *test);
    int (*test_vector_remove)(Test *test);
    int (*test_vector_remove_back)(Test *test);
    int (*test_vector_count)(Test *test);
    int (*test_vector_reset)(Test *test);
    int (*test_vector_to_json)(Test *test);
    int (*test_vector_assign)(Test *test);
    int (*test_vector_new)(Test *test);
    int (*test_vector_search)(Test *test);
    int (*test_vector_get_end_index)(Test *test);
    int (*test_vector_sort)(Test *test);
    int (*test_vector_filter)(Test *test);

    Vector *vector;
};

#endif
