#ifndef __EVENT_THREAD_H__
#define __EVENT_THREAD_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/thread.h>
#include <libobject/event/event_base.h>

typedef struct event_thread_s Event_Thread;

struct event_thread_s{
	Thread parent;

	int (*construct)(Event_Thread *,char *init_str);
	int (*deconstruct)(Event_Thread *);
	int (*set)(Event_Thread *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*add_event)(Event_Thread *, void *event);
    int (*del_event)(Event_Thread *, void *event);

    /*inherited methods*/
    int (*start)(Event_Thread *);
    int (*set_start_routine)(Event_Thread *, void *);
    int (*set_start_arg)(Event_Thread *, void *);

	/*virtual methods reimplement*/
    void *(*start_routine)(void *);

    Event_Base *eb;
    int ctl_read;
    int ctl_write;
    event_t ctl_read_event;
};

#endif
