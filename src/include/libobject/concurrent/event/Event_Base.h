#ifndef __EVENT_BASE_H__
#define __EVENT_BASE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/miscellany/buffer.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Rbtree_Map.h>
#include <libobject/concurrent/event/event.h>
#include <libobject/concurrent/event/signal.h>
#include <libobject/concurrent/event/Rbtree_Timer.h>
#include <libobject/core/Linked_List.h>

extern List *global_event_base_list;

typedef struct event_base_s Event_Base;

struct event_base_s{
    Obj obj;

    int (*construct)(Event_Base *,char *init_str);
    int (*deconstruct)(Event_Base *);
    int (*set)(Event_Base *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*loop)(Event_Base *);
    int (*init)(Event_Base *);
    int (*activate_io)(Event_Base *, int fd, short events); 
    int (*activate_signal)(Event_Base *, int fd, short events); 
    int (*add)(Event_Base *, event_t *e);
    int (*del)(Event_Base *, event_t *e); 

    /*virtual methods reimplement*/
    int (*trustee_io)(Event_Base *, event_t *e);
    int (*reclaim_io)(Event_Base *, event_t *e); 
    /*
     *int (*ctl)(Event_Base *, int fd, int op, short events);
     */
    int (*dispatch)(Event_Base *, struct timeval *tv);

    Timer *timer;
    Map *io_map;
    Iterator *map_iter;
    struct evsig_s evsig;
    int break_flag;

    event_t quit_event;
    event_t coredump_event;
};

int evsig_add(Event_Base *eb, event_t *event);
int evsig_init(Event_Base *eb);
int set_quit_signal(Event_Base* eb);
int evsig_release(Event_Base *eb);
int evsig_socketpair(int fd[2]);
int evsig_del(Event_Base *eb, event_t *event);

#endif
