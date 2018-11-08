#ifndef __WORKER_H__
#define __WORKER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/hash_map.h>
#include <libobject/concurrent/producer.h>
#include <libobject/event/event.h>

typedef struct worker_s Worker;

struct worker_s{
	Obj obj;

	int (*construct)(Worker *,char *init_str);
	int (*deconstruct)(Worker *);
	int (*set)(Worker *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int  (*assign)(Worker *worker, int fd, int ev_events,
                   struct timeval *ev_tv, void *ev_callback,
                   void *ev_arg, void *work_callback);
    int  (*enroll)(Worker *, void *);
    int  (*resign)(Worker *);

    Producer *producer;
    event_t event;
    void *opaque;
    void (*work_callback)(void *);
    int flags;
};

extern Worker *
io_worker(allocator_t *allocator, int fd, 
          struct timeval *ev_tv, void *ev_callback, 
          void *work_callback, Producer *producer, 
          void *opaque);

#endif
