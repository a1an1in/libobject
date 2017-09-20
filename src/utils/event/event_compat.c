#include <stdio.h>
#include <libobject/utils/event/event_compat.h>

struct event_base * event_base_new(void)
{
    Event_Base *eb;
    char buf[2048];
    allocator_t *allocator = allocator_get_default_alloc();
    struct event_base *event_base;

    eb = OBJECT_NEW(allocator, Select_Base, NULL);
    object_dump(eb, "Select_Base", buf, 2048);
    dbg_str(DBG_DETAIL,"Select_Base dump: %s",buf);

    event_base = (struct event_base *) allocator_mem_alloc(allocator, sizeof(struct event_base));
    if (event_base == NULL) {
        return NULL;
    }

    event_base->eb = eb;

    return event_base;
}

int event_assign(event_t *ev,
                 struct event_base *base,
                 int fd,
                 short events,
                 void (*callback)(int, short, void *), void *arg)
{

    ev->ev_fd = fd;
    ev->ev_events = events;
    ev->ev_callback = callback;
    ev->ev_arg = arg;
    ev->ev_base = (void *)base->eb;

    return 0;
}

int event_add(event_t *ev, const struct timeval *tv)
{
    Event_Base *eb = (Event_Base *)ev->ev_base;

    ev->ev_timeout.tv_sec  = tv->tv_sec;
    ev->ev_timeout.tv_usec = tv->tv_usec;
    eb->add(eb, ev);

    return 0;
}

int event_base_dispatch(struct event_base *event_base)
{
    Event_Base *eb = event_base->eb;

    eb->loop(eb);

    return 0;
}

int event_base_distroy(struct event_base *event_base)
{
    allocator_t *allocator = allocator_get_default_alloc();

    object_destroy(event_base->eb);
    allocator_mem_free(allocator, event_base);

    return 0;
}
