#ifndef __MESSAGE_SUBSCRIBER_H__
#define __MESSAGE_SUBSCRIBER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/map.h>
#include <libobject/message/message.h>

typedef struct subscriber_s Subscriber;

typedef struct message_method_s {
    void (*func)(message_t *, void *arg);
    void *arg;
}message_method_t;

struct subscriber_s{
	Obj obj;

	int (*construct)(Subscriber *,char *init_str);
	int (*deconstruct)(Subscriber *);
	int (*set)(Subscriber *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*connect_centor)(Subscriber *subscriber, void *centor);
    int (*subscribe)(Subscriber *subscriber, void *publisher);
    int (*add_method)(Subscriber *subscriber,char *method_name,
                      void (*func)(message_t *, void *), void *arg);

    /*
     *Subscriber *next;
     */
    void *publisher;
    void *centor;
    void (*message_handler)(void *);
    void *opaque;
    void *message;
    Map *method_map;
};

#endif
