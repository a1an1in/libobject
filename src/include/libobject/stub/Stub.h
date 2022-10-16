#ifndef __STUB_H__
#define __STUB_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Stub_s Stub;

struct Stub_s{
	Obj parent;

	int (*construct)(Stub *,char *);
	int (*deconstruct)(Stub *);

	/*virtual methods reimplement*/
	int (*set)(Stub *module, char *attrib, void *value);
    void *(*get)(Stub *, char *attrib);
};

#endif
