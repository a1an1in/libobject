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
    void *(*teardown)(Test *);
    void (*test_get_cstr)(Test *);
    void (*test_append)(Test *);
    void (*test_append_string)(Test *);
    void (*test_len)(Test *);
    void (*test_get_substring)(Test *);
    void (*test_insert)(Test *);
    void (*test_insert_string)(Test *);
    void (*test_split)(Test *);
    void (*test_find)(Test *);
    void (*test_replace)(Test *);
    void (*test_replace_n)(Test *);
    void (*test_empty)(Test *);
    void (*test_ltrim)(Test *);
    void (*test_rtrim)(Test *);

    String *str;
};

#endif
