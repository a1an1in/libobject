#ifndef __EVENT_BASE_H__
#define __EVENT_BASE_H__

#include <stdio.h>
#include <libdbg/debug.h>
#include <libobject/core/obj.h>

typedef struct event_base_s Event_Base;

struct event_base_s{
	Obj obj;

	int (*construct)(Event_Base *,char *init_str);
	int (*deconstruct)(Event_Base *);
	int (*set)(Event_Base *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    void *(*init)(Event_Base *); 
    int (*add)(Event_Base *, int fd, short old, short events, void *fdinfo);
    int (*del)(Event_Base *, int fd, short old, short events, void *fdinfo); 
    int (*ctl)(Event_Base *, int fd, int op, short events);
    int (*dispatch)(Event_Base *, struct timeval *tv);
    void (*dealloc)(Event_Base *);
};

#endif
