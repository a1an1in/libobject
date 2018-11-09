#ifndef __MESSAGE_PUBLISHER_H__
#define __MESSAGE_PUBLISHER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/message/message.h>
#include <libobject/message/centor.h>

typedef struct msg_publisher_s Publisher;

struct msg_publisher_s{
	Obj obj;

	int (*construct)(Publisher *,char *init_str);
	int (*deconstruct)(Publisher *);
	int (*set)(Publisher *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

    int (*connect_centor)(Publisher *, Centor *);
    int (*publish)(Publisher *, message_t *);
    int (*publish_message)(Publisher *publisher, char *what, void *opaque);

    void *centor;
};

#endif
