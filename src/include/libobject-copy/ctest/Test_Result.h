#ifndef __TEST_RESULT_H__
#define __TEST_RESULT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Vector.h>

typedef struct _test_result_s Test_Result;

struct _test_result_s{
	Obj obj;

	int (*construct)(Test_Result *result,char *init_str);
	int (*deconstruct)(Test_Result *result);

	/*virtual methods reimplement*/
	int (*set)(Test_Result *result, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*attribs*/
    Vector *failed_cases;
    Vector *success_cases;
};

#endif
