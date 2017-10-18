#ifndef __EVENT_THREAD_H__
#define __EVENT_THREAD_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/thread.h>

typedef struct event_thread_s Event_Thread;

struct event_thread_s{
	Obj obj;

	int (*construct)(Thread *,char *init_str);
	int (*deconstruct)(Thread *);
	int (*set)(Thread *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*start)(Thread *);
    int (*set_start_routine)(Thread *, void *);
    int (*set_start_arg)(Thread *, void *);

	/*virtual methods reimplement*/
    void *(*start_routine)(void *);
};

#endif
