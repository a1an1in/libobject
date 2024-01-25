#include <stdio.h>
#include <libobject/concurrent/event_api.h>
#include <libobject/concurrent/Producer.h>

/*
 *global event base use producer event base
 */
struct event_base *global_event_base; 

struct event_base * event_base_new(void)
{
    Event_Base *eb;
    char buf[2048];
    allocator_t *allocator = allocator_get_default_instance();
    struct event_base *event_base;

    eb = object_new(allocator, "Select_Base", NULL);
    eb->init(eb);

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
    ev->ev_flags    = 0;

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

    //todo: 这里有个bug， 如果使用的producer的base， 有可能base loop
    //已经在阻塞select了,这里add没有唤醒select，所以需要去获取event thread
    //的sender。但是这个只是兼容接口, 推荐直接使用producer就没有问题。 直接
    //new event_base_new 然后再使用这个接口也没有这个问题。后期可以加上sender。
    eb->add(eb, ev);

    return 0;
}

int event_del(event_t *ev)
{
    Event_Base *eb;
    int ret;

    TRY {
        THROW_IF(ev == NULL || ev->ev_base == NULL, -1);

        eb = (Event_Base *)ev->ev_base;
        eb->del(eb, ev);
    } CATCH (ret) {
    }

    return ret;
}

int event_base_dispatch(struct event_base *event_base)
{
    Event_Base *eb = event_base->eb;

    eb->loop(eb);

    return 0;
}

int event_base_free(struct event_base *event_base)
{
    allocator_t *allocator = allocator_get_default_instance();

    dbg_str(DBG_DETAIL,"event_base_free");
    object_destroy(event_base->eb);
    allocator_mem_free(allocator, event_base);

    return 0;
}

int event_base_init_default_instance()
{
    Producer *producer = producer_get_default_instance();
    allocator_t *allocator = allocator_get_default_instance();
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

int event_base_destroy_default_instance()
{
    allocator_t *allocator = allocator_get_default_instance();
    struct event_base *eb = event_base_get_default_instance();

    allocator_mem_free(allocator, eb);
     
    return 0;
}
struct event_base *event_base_get_default_instance()
{
    return global_event_base;
}