#ifndef __EVENT_BASE_H__
#define __EVENT_BASE_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/map_hash.h>
#include <libobject/event/event.h>
#include <libobject/event/timer_rbtree.h>
#include <libobject/utils/miscellany/buffer.h>

typedef struct event_base_s Event_Base;

/** Indicates that a timeout has occurred.  It's not necessary to pass
 * this flag to event_for new()/event_assign() to get a timeout. */
#define EV_TIMEOUT  0x01
/** Wait for a socket or FD to become readable */
#define EV_READ     0x02
/** Wait for a socket or FD to become writeable */
#define EV_WRITE    0x04
/** Wait for a POSIX signal to be raised*/
#define EV_SIGNAL   0x08
/**
 * Persistent event: won't get removed automatically when activated.
 *
 * When a persistent event with a timeout becomes activated, its timeout
 * is reset to 0.
 */
#define EV_PERSIST  0x10
/** Select edge-triggered behavior, if supported by the backend. */
#define EV_ET       0x20

struct event_base_s{
	Obj obj;

	int (*construct)(Event_Base *,char *init_str);
	int (*deconstruct)(Event_Base *);
	int (*set)(Event_Base *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*loop)(Event_Base *);
    int (*active_io)(Event_Base *, int fd, short events); 
    int (*add)(Event_Base *, event_t *e);
    int (*del)(Event_Base *, event_t *e); 

	/*virtual methods reimplement*/
    int (*add_io)(Event_Base *, event_t *e);
    int (*del_io)(Event_Base *, event_t *e); 
    /*
     *int (*ctl)(Event_Base *, int fd, int op, short events);
     */
    int (*dispatch)(Event_Base *, struct timeval *tv);

    Timer *timer;
    Map *map;
    Iterator *map_iter;
};

#endif
