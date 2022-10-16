#ifndef __SKCIPHER_TEST_H__
#define __SKCIPHER_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>

typedef struct skcipher_test_s Skcipher_Test;

struct skcipher_test_s{
    Test parent;

    int (*construct)(Test *,char *init_str);
    int (*deconstruct)(Test *);
    int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

    /*virtual methods reimplement*/
    int (*setup)(Test *);
    void *(*teardown)(Test *);

};

#endif
