#ifndef __TEST_OPTION_H__
#define __TEST_OPTION_H__

#include <stdio.h>
#include <libobject/core/String.h>

typedef struct Option_s Option;

struct Option_s{
	Obj parent;

	int (*construct)(Option *option,char *init_str);
	int (*deconstruct)(Option *option);

	/*virtual methods reimplement*/
	int (*set)(Option *option, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 

    /*attribs*/
    String *name;
    String *alias;
    String *usage;
    int (*action)(void *, Option *);
    int value_type;
    int int_value;
    String *string_value;
};

#endif
