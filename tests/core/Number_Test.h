#ifndef __NUMBER_TEST_H__
#define __NUMBER_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/core/Number.h>

typedef struct number_test_s Number_Test;

struct number_test_s{
	Test parent;

	int (*construct)(Test *,char *init_str);
	int (*deconstruct)(Test *);
	int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

	/*virtual methods reimplement*/
	int (*setup)(Test *);
    void *(*teardown)(Test *);
    int (*test_int_number)(Number_Test *test);
    int (*test_add)(Number_Test *test);
    int (*test_sub)(Number_Test *test);
    int (*test_mul)(Number_Test *test);
    int (*test_div)(Number_Test *test);
    int (*test_mod)(Number_Test *test);

    /*attrib*/
    Number *number;
};

#endif
