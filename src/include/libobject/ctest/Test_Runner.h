#ifndef __TEST_RUNNER_H__
#define __TEST_RUNNER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Vector.h>
#include <libobject/ctest/Test_Result.h>

typedef struct _test_runner_s Test_Runner;

struct _test_runner_s{
	Obj obj;

	int (*construct)(Test_Runner *runner,char *init_str);
	int (*deconstruct)(Test_Runner *runner);

	/*virtual methods reimplement*/
	int (*set)(Test_Runner *runner, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*set_white_list)(Test_Runner *runner, char *list);
	int (*start)(Test_Runner *runner);
	int (*run_test)(Test_Runner *runner, char *test_class_name);
    int (*is_testcase_in_white_list)(Test_Runner *runner, char *testcase);

    /*attribs*/
    Test_Result *result;
    Vector *white_list;
    uint8_t set_white_list_flag;
};

#endif
