#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct system_s System;

struct system_s{
	Obj obj;

	int (*construct)(System *system,char *init_str);
	int (*deconstruct)(System *system);
	int (*set)(System *system, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    char *value;
    int value_max_len;
    int value_len;
};

#endif
