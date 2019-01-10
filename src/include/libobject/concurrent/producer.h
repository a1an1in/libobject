#ifndef __PRODUCER_H__
#define __PRODUCER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/hash_map.h>
#include <libobject/core/list.h>
#include <libobject/concurrent/dispatcher.h>
#include <libobject/event/event_thread.h>

typedef struct producer_s Producer;

struct producer_s{
	Event_Thread parent;

	int (*construct)(Producer *,char *init_str);
	int (*deconstruct)(Producer *);
	int (*set)(Producer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*add_worker)(Producer *, void *);
    int (*del_worker)(Producer *, void *);
    int (*add_dispatcher)(Producer *, void *);
    int (*del_dispatcher)(Producer *, void *);

    /*inherited methods*/
    int (*start)(Producer *);

    Dispatcher *dispatcher;
    List *workers;
};

Producer *global_get_default_producer();

#endif
