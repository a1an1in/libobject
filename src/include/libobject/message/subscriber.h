#ifndef __MESSAGE_SUBSCRIBER_H__
#define __MESSAGE_SUBSCRIBER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

typedef struct subscriber_s Subscriber;

struct subscriber_s{
	Obj obj;

	int (*construct)(Subscriber *,char *init_str);
	int (*deconstruct)(Subscriber *);
	int (*set)(Subscriber *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*connect_centor)(Subscriber *subscriber, void *centor);
    int (*subscribe)(Subscriber *subscriber, void *publisher);
    int (*add_message_handler)(Subscriber *subscriber, void (*func)(void *));
    int (*add_message_handler_arg)(Subscriber *subscriber, void *arg);

    /*
     *Subscriber *next;
     */
    void *publisher;
    void *centor;
    void (*message_handler)(void *);
    void *message_handler_arg;
    void *message;
};

#endif
