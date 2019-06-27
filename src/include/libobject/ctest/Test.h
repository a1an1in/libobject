#ifndef __TEST_H__
#define __TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/ctest/Test_Case_Result.h>

typedef struct _test_s Test;

#define Init_Test_Case(test) \
    ((Test *)test)->line = __LINE__;\
    ((Test *)test)->file = __FILE__;

#define ASSERT_EQUAL(test, peer1, peer2, len) \
({\
    int __ret;\
    ((Test *)test)->line = __LINE__;\
    ((Test *)test)->file = __FILE__;\
    __ret = memcmp(peer1, peer2, len) == 0;\
    ((Test *)test)->ret = __ret;\
    __ret;\
})

struct _test_s{
	Obj obj;

	int (*construct)(Test *test,char *init_str);
	int (*deconstruct)(Test *test);

	/*virtual methods reimplement*/
	int (*set)(Test *test, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
	int (*setup)(Test *test);
    void *(*teardown)(Test *test);

    /*attribs*/
    char *file;
    int line;
    int ret;
};

#endif