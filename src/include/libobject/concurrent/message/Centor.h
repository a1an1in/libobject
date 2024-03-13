#ifndef __MESSAGE_CENTOR_H__
#define __MESSAGE_CENTOR_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/concurrent/Worker.h>
#include <libobject/core/Queue.h>
#include <libobject/core/Map.h>
#include <libobject/core/Lock.h>
#include <libobject/core/io/Socket.h>

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
