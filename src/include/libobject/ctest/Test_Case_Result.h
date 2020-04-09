#ifndef __TEST_CASE_RESULT_H__
#define __TEST_CASE_RESULT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>

typedef struct _test_case_result_s Test_Case_Result;

struct _test_case_result_s{
    Obj obj;

    int (*construct)(Test_Case_Result *test,char *init_str);
    int (*deconstruct)(Test_Case_Result *test);

    /*virtual methods reimplement*/
    int (*set)(Test_Case_Result *test, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*attribs*/
    String *file;
    int line;
    int result;
};

#endif
