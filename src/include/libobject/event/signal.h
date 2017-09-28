#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include <stdio.h>
#include <libobject/event/event.h>
#include <libobject/utils/miscellany/buffer.h>
#include <libobject/utils/data_structure/rbtree_map.h>

struct evsig_s{
    int fd_snd, fd_rcv;
    event_t fd_rcv_event;

    rbtree_map_t *sig_map;
};


#endif
