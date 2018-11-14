#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include <stdio.h>
#include <libobject/event/event.h>
#include <libobject/core/utils/miscellany/buffer.h>
#include <libobject/core/map.h>

struct evsig_s{
    int fd_snd, fd_rcv;
    event_t fd_rcv_event;

    Map *map;
    List *list;
};

/*
 *int evsig_del(Event_Base *eb, event_t *event);
 */

#endif
