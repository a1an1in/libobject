#ifndef __RBTREE_TIMER_H__
#define __RBTREE_TIMER_H__

#include <stdio.h> 
#include <sys/time.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/data_structure/rbtree_map.h>
#include <libobject/core/obj.h>
#include <libobject/event/timer.h>

typedef struct rbtree_timer_s Rbtree_Timer;

struct rbtree_timer_s{
	Timer parent;

	int (*construct)(Rbtree_Timer *,char *init_str);
	int (*deconstruct)(Rbtree_Timer *);
	int (*set)(Rbtree_Timer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*add)(Rbtree_Timer *, event_t *e);
    int (*del)(Rbtree_Timer *, event_t *e); 
    int (*remove)(Rbtree_Timer *, event_t *e); 
    int (*timeout_next)(Rbtree_Timer *, struct timeval **tv);
    event_t * (*first)(Rbtree_Timer *); 

    rbtree_map_t *map;
};

#endif
