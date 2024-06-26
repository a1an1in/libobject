/**
 * @file event_base.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-13
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/event/Event_Base.h>

List *global_event_base_list;

static int __construct(Event_Base *eb, char *init_str)
{
    allocator_t *allocator = eb->obj.allocator;
    List **list = &global_event_base_list;

    dbg_str(EV_VIP, "Event_Base construct, eb addr:%p", eb);

    eb->break_flag = 0;
    eb->timer  = object_new(allocator, "Rbtree_Timer", NULL);
    eb->io_map = object_new(allocator, "RBTree_Map", NULL);

    dbg_str(EV_DETAIL, "base addr:%p, io_map addr :%p, timer:%p", 
            eb, eb->io_map, eb->timer);

    if (*list == NULL) {
        *list = object_new(allocator, "Linked_List", NULL);
    }

    (*list)->add_back(*list, eb);

    return 0;
}

static int __deconstrcut(Event_Base *eb)
{
    List *list = global_event_base_list;

    dbg_str(EV_VIP, "Event_Base deconstruct, eb addr:%p", eb);

    evsig_release(eb);
    object_destroy(eb->timer);
    object_destroy(eb->io_map);

    if (list != NULL && list->count(list) == 1) {
        dbg_str(EV_DETAIL, "destroy global event base list");
        object_destroy(list);
    } else {
        list->remove_element(list, eb);
        dbg_str(EV_DETAIL, "remove global_event_base_list, count=%d", list->count(list));
    }

    object_destroy(eb->signal_service);

    return 0;
}

static int __init(Event_Base *eb)
{
    dbg_str(DBG_DETAIL, "Event_Base, init");
    return evsig_init(eb);
}

static int __add(Event_Base *eb, event_t *event)
{
    Timer *timer = eb->timer;
    Map *io_map  = eb->io_map;
    int fd       = event->ev_fd;

    if (event->ev_events & EV_SIGNAL) {
        dbg_str(EV_VIP, "event base add signal, event base:%p signal:%d, event addr:%p", 
                eb, fd, event);
        evsig_add(eb, event);
    } else {
        dbg_str(EV_VIP, "event base add io, fd:%d, event addr:%p", fd, event);
        event->ev_tv = event->ev_timeout;
        dbg_str(EV_DETAIL, "add fd =%d into io map", fd);
        io_map->add(io_map, event->ev_fd, event);

        eb->trustee_io(eb, event);
        timer->add(timer, event);
    }

    return (0);
}

static int __del(Event_Base *eb, event_t *event) 
{
    Timer *timer = eb->timer;
    int fd       = event->ev_fd;
    Map *io_map  = eb->io_map;
    Map *sig_map = eb->evsig.map;
    int ret;

    if (event->ev_events & EV_SIGNAL) {
        evsig_del(eb, event);
    } else {
        eb->reclaim_io(eb, event);
        timer->del(timer, event);

        //del fd in map
        ret = io_map->del(io_map, fd);
        if (ret < 0) {
            dbg_str(EV_WARN, "event base del fd = %d, but not found fd in io_map, ret=%d", 
                    fd, ret);
        } else {
            dbg_str(EV_VIP, "event base del io, fd = %d, evnt addr:%p", fd, event);
        }
    }

    return 0;
}

static int __activate_io(Event_Base *eb, int fd, short events)
{
    Timer *timer = eb->timer;
    Map *io_map  = eb->io_map;
    struct timeval tv;
    char *p;
    char buf[255];
    int len;
    event_t *event;
    int ret;

    dbg_str(EV_DETAIL, "event base active io event, fd = %d", fd);

    ret = io_map->search(io_map, fd, (void **)&event);
    dbg_str(EV_DETAIL, "search ret=%d", ret);

    if (ret <= 0) {
        dbg_str(EV_INFO, "not found fd = %d in io_map, ret=%d", fd, ret);
    } else {
        dbg_str(EV_DETAIL, "event addr:%p, ev_callback=%p", 
                event, event->ev_callback);

        event->ev_callback(event->ev_fd, events, event->ev_arg);

        if (event->ev_events & EV_PERSIST) {
            timer->del(timer, event);
            event->ev_timeout = event->ev_tv;
            timer->add(timer, event);
        } else {
            dbg_str(EV_DETAIL, "del event");
            eb->del(eb, event);
        } 
    }

    return 0;
}

static void __signal_list_for_each_callback(void *element)
{
    event_t *event = (event_t *)element;
    event->ev_callback(event->ev_fd, 0, event->ev_arg);

    if (!(event->ev_events & EV_PERSIST)) {
        evsig_del(event->ev_base, event);
    };
}

static int __activate_signal(Event_Base *eb, int fd, short events)
{
    Map *map = eb->evsig.map;
    List *list = eb->evsig.list;
    event_t *event = NULL;
    char buf[16];
    char *p = NULL;
    int ret;

    dbg_str(EV_DETAIL, "event base active signal event, event base:%p, signal = %d, ncount=%d",
            eb, fd, events);

    list->reset(list);

    map->search_all_same_key(map, fd, list);
    if (list->count(list) != 0) {
        dbg_str(EV_WARN, "activate_signal, list count=%d, signal=%d", list->count(list), fd);
        list->for_each(list, __signal_list_for_each_callback);
    } else {
        dbg_str(EV_WARN, "activate_signal, get event addr error");
        return -1;
    }

    return 0;
}

static int __process_timeout_events(Event_Base *eb)
{
    Timer *timer = eb->timer;
    struct timeval now, search;
    event_t *event;

    event = timer->first(timer);
    if (event == NULL) {
        return -1;
    }

    timeval_now(&now, NULL);
    if (timeval_cmp(&now, &event->ev_timeout) >= 0) {
        dbg_str(EV_DETAIL, "process_timeout, event addr:%p", event);
        event->ev_callback(event->ev_fd, 0, event->ev_arg);
        if (event->ev_events & EV_PERSIST) {
            timer->remove(timer, event);
            event->ev_timeout = event->ev_tv;
            timer->add(timer, event);
        } else {
            timer->del(timer, event);
        }
    }

    return 0;
}

static int __loop(Event_Base *eb)
{
    Timer *timer = eb->timer;
    struct timeval tv, *tv_p;

    while (eb->break_flag == 0) {
        tv_p = &tv;
        timer->timeout_next(timer, &tv_p);
        eb->dispatch(eb, tv_p);
        __process_timeout_events(eb);
    }

    dbg_str(DBG_SUC, "break Event_Base loop");
}

static class_info_entry_t event_base_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj), 
    Init_Nfunc_Entry(1 , Event_Base, construct, __construct), 
    Init_Nfunc_Entry(2 , Event_Base, deconstruct, __deconstrcut), 
    Init_Vfunc_Entry(3 , Event_Base, set, NULL), 
    Init_Vfunc_Entry(4 , Event_Base, init, __init), 
    Init_Vfunc_Entry(5 , Event_Base, loop, __loop), 
    Init_Vfunc_Entry(6 , Event_Base, activate_io, __activate_io), 
    Init_Vfunc_Entry(7 , Event_Base, activate_signal, __activate_signal), 
    Init_Vfunc_Entry(8 , Event_Base, add, __add), 
    Init_Vfunc_Entry(9 , Event_Base, del, __del), 
    Init_Vfunc_Entry(10, Event_Base, trustee_io, NULL), 
    Init_Vfunc_Entry(11, Event_Base, reclaim_io, NULL), 
    Init_Vfunc_Entry(12, Event_Base, dispatch, NULL),
    Init_Str___Entry(13, Event_Base, signal_service, NULL),
    Init_End___Entry(14, Event_Base), 
};
REGISTER_CLASS(Event_Base, event_base_class_info);

void test_obj_eb()
{
    Event_Base *eb;
    allocator_t *allocator = allocator_get_default_instance();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(EV_SUC, "configurator_t addr:%p", c);
    cfg_config_str(c, "/Event_Base", "name", "alan eb") ;  

    eb = OBJECT_NEW(allocator, Event_Base, c->buf);

    object_dump(eb, "Event_Base", buf, 2048);
    dbg_str(EV_DETAIL, "Event_Base dump: %s", buf);

    object_destroy(eb);
    cfg_destroy(c);
}
