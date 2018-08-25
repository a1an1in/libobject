#ifndef __CONSUMER_H__
#define __CONSUMER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct consumer_s Consumer;

struct consumer_s{
	Obj obj;

	int (*construct)(Consumer *,char *init_str);
	int (*deconstruct)(Consumer *);
	int (*set)(Consumer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*add_dispatcher)(Consumer *, void *);
    int (*del_dispatcher)(Consumer *, void *);

};

#endif
