#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct mould_s Mould;

struct mould_s{
	Obj obj;

	int (*construct)(Mould *,char *init_str);
	int (*deconstruct)(Mould *);
	int (*set)(Mould *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
