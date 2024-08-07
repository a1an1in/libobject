#ifndef __PRODUCER_H__
#define __PRODUCER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Hash_Map.h>
#include <libobject/core/List.h>
#include <libobject/concurrent/Dispatcher.h>
#include <libobject/concurrent/event/Event_Thread.h>

typedef struct producer_s Producer;

struct producer_s{
	Event_Thread parent;

	int (*construct)(Producer *,char *init_str);
	int (*deconstruct)(Producer *);

	/*virtual methods reimplement*/
    int (*set)(Producer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*add_worker)(Producer *, void *);
    int (*del_worker)(Producer *, void *);
    int (*add_dispatcher)(Producer *, void *);
    int (*del_dispatcher)(Producer *, void *);

    /*inherited methods*/
    int (*start)(Producer *);
    int (*close)(Producer *);

    Dispatcher *dispatcher;
    List *workers;
};

extern Producer *producer_get_default_instance();
extern int producer_init_default_instance(char *event_thread_service, char *event_signal_service);
extern int producer_destroy_default_instance();

#endif
