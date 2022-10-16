#ifndef __MODULE_TEST_H__
#define __MODULE_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include <libobject/core/Object_Cache.h>

typedef struct object_cache_test_s Object_Cache_Test;

struct object_cache_test_s{
    Test parent;

    int (*construct)(Test *,char *init_str);
    int (*deconstruct)(Test *);
    int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

    /*virtual methods reimplement*/
    int (*setup)(Test *);
    void *(*teardown)(Test *);
    int (*test_memery_leak)(Object_Cache_Test *test);

    Object_Cache *cache;
};

#endif
