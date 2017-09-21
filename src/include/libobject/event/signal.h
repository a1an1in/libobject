#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include <stdio.h>
#include <libobject/event/event.h>
#include <libobject/core/map_hash.h>

struct evsig_s{
    int fd_snd, fd_rcv;
    event_t fd_rcv_event;
    Map *sig_map;
    Iterator *sig_map_iter;
};



#endif
