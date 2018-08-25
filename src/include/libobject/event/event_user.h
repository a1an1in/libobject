#ifndef __EVENT_USER_H__
#define __EVENT_USER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/event/event.h>

typedef struct event_user_s Event_User;

struct event_user_s{
	Obj obj;

	int (*construct)(Event_User *,char *init_str);
	int (*deconstruct)(Event_User *);
	int (*set)(Event_User *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

};

#endif
