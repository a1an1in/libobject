#ifndef __MODULE_H__
#define __MODULE_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Module_s Module;

struct Module_s{
	Obj parent;

	int (*construct)(Module *,char *);
	int (*deconstruct)(Module *);

	/*virtual methods reimplement*/
	int (*set)(Module *module, char *attrib, void *value);
    void *(*get)(Module *, char *attrib);
    char *(*to_json)(Module *); 
};

#endif
