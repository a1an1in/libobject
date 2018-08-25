#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct dispatcher_s Dispatcher;

struct dispatcher_s{
	Obj obj;

	int (*construct)(Dispatcher *,char *init_str);
	int (*deconstruct)(Dispatcher *);
	int (*set)(Dispatcher *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*add_producer)(Dispatcher *, void *);
    int (*del_producer)(Dispatcher *, void *);
    int (*add_consumer)(Dispatcher *, void *);
    int (*del_consumer)(Dispatcher *, void *);
};

#endif
