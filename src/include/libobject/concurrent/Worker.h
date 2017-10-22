#ifndef __WORKER_H__
#define __WORKER_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/hash_map.h>

typedef struct worker_s Worker;

struct worker_s{
	Obj obj;

	int (*construct)(Worker *,char *init_str);
	int (*deconstruct)(Worker *);
	int (*set)(Worker *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
};

#endif
