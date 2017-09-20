#ifndef __EVENT_H__
#define __EVENT_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>

typedef struct event event_t;

struct event{
    uint32_t ev_fd;
    void *fdinfo;
    void *ev_base;
    short ev_events;
    short ev_oldevents;
    short ev_res;       /* result passed to event callback */
    short ev_flags;
    uint8_t ev_pri;     /* smaller numbers are higher priority */
    uint8_t ev_closure;
    struct timeval ev_timeout, ev_tv;

    void (*ev_callback)(int fd, short, void *arg);
    void *ev_arg;
};

#endif
