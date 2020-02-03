#ifndef __COMPOSITE_OBJ_H__
#define __COMPOSITE_OBJ_H__

#include <stdio.h>
#include <libobject/core/String.h>
#include <libobject/core/Vector.h>
#include <libobject/core/Number.h>

typedef struct Composite_Obj_s Composite_Obj;

struct Composite_Obj_s{
	Obj parent;

	int (*construct)(Composite_Obj *obj,char *init_str);
	int (*deconstruct)(Composite_Obj *obj);

	/*virtual methods reimplement*/
	int (*set)(Composite_Obj *obj, char *attrib, void *value);
    void *(*get)(Composite_Obj *obj, char *attrib);
    char *(*to_json)(Composite_Obj *obj); 

    int help;
    String *name;
    Vector *vector;
    Number *num;
};

#endif
