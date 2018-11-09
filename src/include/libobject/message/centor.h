#ifndef __MESSAGE_CENTOR_H__
#define __MESSAGE_CENTOR_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/socket/socket.h>
#include <libobject/concurrent/worker.h>
#include <libobject/core/queue.h>
#include <libobject/core/map.h>
#include <libobject/core/lock.h>

typedef struct centor_s Centor;

struct centor_s{
	Obj obj;

	int (*construct)(Centor *,char *init_str);
	int (*deconstruct)(Centor *);
	int (*set)(Centor *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

    Socket *s, *c;
    Worker *worker;
    Queue *message_queue;
    Map *subscriber_map;
    /*
     *Lock *lock;
     */
};

#endif
