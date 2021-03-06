#ifndef __COMPOSITE_OBJ_TEST_H__
#define __COMPOSITE_OBJ_TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ctest/Test.h>
#include "Composite_Obj.h"

typedef struct composite_obj_test_s Composite_Obj_Test;

struct composite_obj_test_s{
    Test parent;

    int (*construct)(Test *,char *init_str);
    int (*deconstruct)(Test *);
    int (*set)(Test *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);

    /*virtual methods reimplement*/
    int (*setup)(Test *);
    void *(*teardown)(Test *);
    void (*test_marshal_composite_obj)(Test *);
    void (*test_unmarshal_composite_obj)(Test *);
    void (*test_override_virtual_funcs)(Test *);

    Composite_Obj *obj;
};

#endif
