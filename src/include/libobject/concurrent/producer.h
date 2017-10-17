#ifndef __PRODUCER_H__
#define __PRODUCER_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/map_hash.h>

typedef struct producer_s Producer;

struct producer_s{
	Obj obj;

	int (*construct)(Producer *,char *init_str);
	int (*deconstruct)(Producer *);
	int (*set)(Producer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
