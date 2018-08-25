#ifndef __EPOLL_BASE_H__
#define __EPOLL_BASE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/event/event_base.h>

typedef struct epoll_base_s Epoll_Base;

struct epoll_base_s{
	Event_Base base;

	int (*construct)(Epoll_Base *,char *init_str);
	int (*deconstruct)(Epoll_Base *);
	int (*set)(Epoll_Base *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    void *(*init)(Epoll_Base *); 
    int (*add)(Event_Base *, event_t *e);
    int (*del)(Event_Base *, event_t *e); 
    int (*ctl)(Epoll_Base *, int fd, int op, short events);
    int (*dispatch)(Epoll_Base *, struct timeval *tv);
    void (*dealloc)(Epoll_Base *);
};

#endif
