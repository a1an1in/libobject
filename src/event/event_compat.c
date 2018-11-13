#include <stdio.h>
#include <libobject/event/event_compat.h>
#include <libobject/concurrent/producer.h>


struct event_base *global_event_base;

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
    ev->ev_fd       = fd;
    ev->ev_events   = events;
    ev->ev_callback = callback;
    ev->ev_arg      = arg;
    ev->ev_base     = (void *)base->eb;

    dbg_str(DBG_DETAIL,"ev_callback=%p", ev->ev_callback);

    return 0;
}

int event_add(event_t *ev, const struct timeval *tv)
{
    Event_Base *eb = (Event_Base *)ev->ev_base;

    dbg_str(DBG_DETAIL,"event add, fd=%d", ev->ev_fd);

    if (tv != NULL) {
        ev->ev_timeout.tv_sec  = tv->tv_sec;
        ev->ev_timeout.tv_usec = tv->tv_usec;
    }

    eb->add(eb, ev);

    return 0;
}

int event_del(event_t *ev)
{
    Event_Base *eb = (Event_Base *)ev->ev_base;

    eb->del(eb, ev);

    return 0;
}

int event_base_dispatch(struct event_base *event_base)
{
    Event_Base *eb = event_base->eb;

    eb->loop(eb);

    return 0;
}

int event_base_free(struct event_base *event_base)
{
    allocator_t *allocator = allocator_get_default_alloc();

    dbg_str(DBG_DETAIL,"event_base_free");
    object_destroy(event_base->eb);
    allocator_mem_free(allocator, event_base);

    return 0;
}

struct event_base *get_default_event_base()
{
    return global_event_base;
}

int default_event_base_constructor()
{
    Producer *producer = global_get_default_producer();
    allocator_t *allocator = allocator_get_default_alloc();
    struct event_base *event_base;

    event_base = (struct event_base *) allocator_mem_alloc(allocator,
                                                           sizeof(struct event_base));
    if (event_base == NULL) {
        return -1;
    }

    event_base->eb = producer->parent.eb;
    global_event_base = event_base;

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_EVBASE, 
                   default_event_base_constructor);

int default_event_base_destructor()
{
    allocator_t *allocator = allocator_get_default_alloc();
    struct event_base *eb = get_default_event_base();

    allocator_mem_free(allocator, eb);
     
    return 0;
}
REGISTER_DTOR_FUNC(REGISTRY_DTOR_PRIORITY_EVBASE, 
                   default_event_base_destructor);
