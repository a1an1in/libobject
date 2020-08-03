#ifndef __STRING_TEST_H__
#define __STRING_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/core/Array_Stack.h>

typedef struct string_test_s String_Test;

struct string_test_s{
    Test parent;

    int (*construct)(Test *,char *init_str);
    int (*deconstruct)(Test *);
    int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

    /*virtual methods reimplement*/
    int (*setup)(Test *);
    int *(*teardown)(Test *);
    int (*test_get_cstr)(Test *);
    int (*test_sprintf)(String_Test *test);
    int (*test_append)(Test *);
    int (*test_append_string)(Test *);
    int (*test_len)(Test *);
    int (*test_get_substring)(Test *);
    int (*test_insert)(Test *);
    int (*test_insert_string)(Test *);
    int (*test_split)(Test *);
    int (*test_find)(Test *);
    int (*test_replace)(Test *);
    int (*test_empty)(Test *);
    int (*test_ltrim)(Test *);
    int (*test_rtrim)(Test *);

    String *str;
};

#endif
