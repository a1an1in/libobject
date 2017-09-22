#ifndef __EVENT_COMPAT_H__
#define __EVENT_COMPAT_H__

#include <stdio.h>
#include <libobject/event/select_base.h>

struct event_base{
    Event_Base *eb;
};

struct event_base * event_base_new(void);
int event_assign(event_t *ev,
                 struct event_base *base,
                 int fd,
                 short events,
                 void (*callback)(int, short, void *), void *arg);

int event_add(event_t *ev, const struct timeval *tv);
int event_base_dispatch(struct event_base *event_base);
int event_base_free(struct event_base *event_base);

#endif
