#ifndef __SELECT_BASE_H__
#define __SELECT_BASE_H__

#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/event/event_base.h>

typedef struct select_base_s Select_Base;

struct select_base_s{
	Event_Base base;

	int (*construct)(Select_Base *,char *init_str);
	int (*deconstruct)(Select_Base *);
	int (*set)(Select_Base *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*add)(Event_Base *, event_t *e);
    int (*del)(Event_Base *, event_t *e); 
    int (*ctl)(Select_Base *, int fd, int op, short events);
    int (*dispatch)(Select_Base *, struct timeval *tv);

    /*inherit metheod from parent*/
    int (*active_io)(Event_Base *, int fd, short events); 

    int maxfdp; /*max fd plus 1*/

    fd_set event_readset_in;
    fd_set event_writeset_in;
    fd_set event_readset_out;
    fd_set event_writeset_out;
};

#endif
