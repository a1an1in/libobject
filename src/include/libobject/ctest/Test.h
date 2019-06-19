#ifndef __TEST_H__
#define __TEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>

typedef struct _test_s Test;

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
};

#endif
