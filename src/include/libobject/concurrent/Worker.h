#ifndef __WORKER_H__
#define __WORKER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Hash_Map.h>
#include <libobject/concurrent/Producer.h>
#include <libobject/concurrent/event/event.h>
#include <libobject/core/io/Socket.h>

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
    void *task;
    Socket *socket;
    int flags;
    struct timeval lasttime;  //定时器用于记录上次运行worker的时间。
};

#endif
