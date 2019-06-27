#ifndef __ARGUMENT_ARGUMENT_H__
#define __ARGUMENT_ARGUMENT_H__

#include <stdio.h>
#include <libobject/core/String.h>

typedef struct Argument_s Argument;

struct Argument_s{
	Obj parent;

	int (*construct)(Argument *arg,char *init_str);
	int (*deconstruct)(Argument *arg);

	/*virtual methods reimplement*/
	int (*set)(Argument *arg, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 

    /*attribs*/
    String *usage;
    String *value;
};

#endif
