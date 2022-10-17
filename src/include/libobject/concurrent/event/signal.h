#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include <stdio.h>
#include <libobject/concurrent/event/event.h>
#include <libobject/core/utils/miscellany/buffer.h>
#include <libobject/core/Map.h>
#include <libobject/core/io/Socket.h>

struct evsig_s{
    Socket *sender;
    Socket *receiver;

    event_t signal_event;

    Map *map;
    List *list;
};

/*
 *int evsig_del(Event_Base *eb, event_t *event);
 */

#endif
