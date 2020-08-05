#ifndef __TEST_H__
#define __TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/ctest/Test_Case_Result.h>
#include <libobject/core/try.h>

typedef struct _test_s Test;

#define ASSERT_EQUAL(test, peer1, peer2, len) \
({\
    int __ret;\
    ((Test *)test)->line = __LINE__;\
    ((Test *)test)->file = __FILE__;\
    __ret = (memcmp(peer1, peer2, len) == 0);\
    ((Test *)test)->ret = __ret;\
    RETURN_IF(__ret != 1, -1);\
    !!__ret;\
})

#define ASSERT_NUM_GREAT(test, peer1, peer2) \
({\
    int __ret;\
    ((Test *)test)->line = __LINE__;\
    ((Test *)test)->file = __FILE__;\
    __ret = peer1 > peer2 ? 1 : 0;\
    ((Test *)test)->ret = __ret;\
    RETURN_IF(__ret != 1, -1);\
    __ret;\
})

#define ASSERT_NOT_EQUAL(test, peer1, peer2, len) \
({\
    int __ret;\
    ((Test *)test)->line = __LINE__;\
    ((Test *)test)->file = __FILE__;\
    __ret = memcmp(peer1, peer2, len) != 0;\
    ((Test *)test)->ret = __ret;\
    RETURN_IF(__ret != 1, -1);\
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
