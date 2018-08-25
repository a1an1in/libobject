#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/event/event.h>

typedef struct timer_s Timer;

struct timer_s{
	Obj obj;

	int (*construct)(Timer *,char *init_str);
	int (*deconstruct)(Timer *);
	int (*set)(Timer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*add)(Timer *, event_t *e);
    int (*del)(Timer *, event_t *e); 
    int (*remove)(Timer *, event_t *e); 
    event_t * (*first)(Timer *); 
    int (*timeout_next)(Timer *, struct timeval **tv);
};

#endif
