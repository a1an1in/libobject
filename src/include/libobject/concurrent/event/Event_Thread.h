#ifndef __EVENT_THREAD_H__
#define __EVENT_THREAD_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Thread.h>
#include <libobject/core/Queue.h>
#include <libobject/concurrent/event/Event_Base.h>
#include <libobject/core/io/Socket.h>

typedef struct event_thread_s Event_Thread;

enum event_thread_state_e{
    EVTHREAD_STATE_UNINITED = -1,
    EVTHREAD_STATE_INITED,
    EVTHREAD_STATE_RUNNING,
    EVTHREAD_STATE_DESTROYED,
};
struct event_thread_s{
    Thread parent;

    int (*construct)(Event_Thread *,char *init_str);
    int (*deconstruct)(Event_Thread *);

    /*inherited methods*/
    int (*set)(Event_Thread *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*add_event)(Event_Thread *, void *event);
    int (*del_event)(Event_Thread *, void *event);
    int (*start)(Event_Thread *);
    void (*stop)(Event_Thread*);
    int (*set_start_routine)(Event_Thread *, void *);
    int (*set_start_arg)(Event_Thread *, void *);

    /*virtual methods reimplement*/
    void *(*start_routine)(void *);

    Event_Base *eb;
    event_t notifier_event;
    Queue *ev_queue;
    Socket *s, *c;
    int flags;
};

#endif
