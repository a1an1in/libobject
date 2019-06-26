#ifndef __COMPOSITE_OBJ_H__
#define __COMPOSITE_OBJ_H__

#include <stdio.h>
#include <libobject/core/String.h>

typedef struct Composite_Obj_s Composite_Obj;

struct Composite_Obj_s{
	Obj parent;

	int (*construct)(Obj *obj,char *init_str);
	int (*deconstruct)(Obj *obj);
	int (*set)(Obj *obj, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    char *(*to_json)(void *obj); 

	/*virtual methods reimplement*/

    int help;
    String *option;
};

#endif
