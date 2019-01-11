#ifndef __EVENT_THREAD_H__
#define __EVENT_THREAD_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/thread.h>
#include <libobject/core/queue.h>
#include <libobject/event/event_base.h>
#include <libobject/net/socket/socket.h>

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
	int (*set)(Event_Thread *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*add_event)(Event_Thread *, void *event);
    int (*del_event)(Event_Thread *, void *event);

    /*inherited methods*/
    int (*start)(Event_Thread *);
    void (*detach)(Event_Thread*);
    int (*set_start_routine)(Event_Thread *, void *);
    int (*set_start_arg)(Event_Thread *, void *);

	/*virtual methods reimplement*/
    void *(*start_routine)(void *);

    Event_Base *eb;
    event_t server_socket_event;
    Queue *ev_queue;
    Socket *s, *c;
    int flags;
};

#endif
