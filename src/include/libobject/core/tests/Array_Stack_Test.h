#ifndef __ARRAY_STACK_TEST_H__
#define __ARRAY_STACK_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/core/Array_Stack.h>

typedef struct array_stack_test_s Array_Stack_Test;

struct array_stack_test_s{
    Test parent;

    int (*construct)(Test *,char *init_str);
    int (*deconstruct)(Test *);
    int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

    /*virtual methods reimplement*/
    int (*setup)(Test *);
    void *(*teardown)(Test *);
    void (*test_push)(Test *, void *element);
    void (*test_pop)(Test *, void **element);
    void (*test_count)(Test *);

    Array_Stack *stack;
};

#endif
