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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/event_base.h>

List *global_event_base_list;

static int __construct(Event_Base *eb,char *init_str)
{
    allocator_t *allocator = eb->obj.allocator;
    List **list = &global_event_base_list;
    configurator_t * c;
    char buf[2048];

    dbg_str(EV_DETAIL,"eb construct, eb addr:%p",eb);

    eb->break_flag = 0;

    eb->timer = OBJECT_NEW(allocator, Rbtree_Timer,NULL);

    c = cfg_alloc(allocator); 
    cfg_config_num(c, "/RBTree_Map", "key_size", sizeof(int)); 

    eb->io_map  = OBJECT_NEW(allocator, RBTree_Map, c->buf);

    dbg_str(EV_DETAIL,"base addr:%p, io_map addr :%p,timer:%p",
            eb, eb->io_map, eb->timer);

    if (*list == NULL) {
        *list = OBJECT_NEW(allocator, Linked_List, NULL);
    }

    (*list)->add_back(*list, eb);

    cfg_destroy(c);

    return 0;
}

static int __deconstrcut(Event_Base *eb)
{
    List *list = global_event_base_list;

    dbg_str(EV_DETAIL,"*****eb deconstruct,eb addr:%p",eb);

    object_destroy(eb->timer);
    object_destroy(eb->io_map);

    if (list != NULL && list->count(list) == 1) {
        dbg_str(EV_DETAIL,"destroy global event base list");
        object_destroy(list);
    } else {
        list->remove_element(list, eb);
        dbg_str(EV_DETAIL,"remove global_event_base_list, count=%d", list->count(list));
    }

    return 0;
}

static int __set(Event_Base *eb, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        eb->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        eb->get = value;
    } else if (strcmp(attrib, "loop") == 0) {
        eb->loop = value;
    } else if (strcmp(attrib, "add") == 0) {
        eb->add = value;
    } else if (strcmp(attrib, "del") == 0) {
        eb->del = value;
    } else if (strcmp(attrib, "activate_io") == 0) {
        eb->activate_io = value;
    } else if (strcmp(attrib, "activate_signal") == 0) {
        eb->activate_signal = value;
    }
    else if (strcmp(attrib, "construct") == 0) {
        eb->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        eb->deconstruct = value;
    } else if (strcmp(attrib, "trustee_io") == 0) {
        eb->trustee_io = value;
    } else if (strcmp(attrib, "reclaim_io") == 0) {
        eb->reclaim_io = value;
    } else if (strcmp(attrib, "dispatch") == 0) {
        eb->dispatch = value;
    } else {
        dbg_str(EV_DETAIL,"eb set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Event_Base *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"eb get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __add(Event_Base *eb, event_t *event)
{
    Timer *timer = eb->timer;
    Map *io_map  = eb->io_map;
    int fd       = event->ev_fd;
    event_t *new_event;

    dbg_str(EV_DETAIL,"base addr:%p, io_map addr :%p, timer:%p, event:%p, ev_arg:%p",
            eb, eb->io_map, eb->timer, event, event->ev_arg);

    if (event->ev_events & EV_SIGNAL) {
        evsig_add(eb, event);
    } else {
        event->ev_tv = event->ev_timeout;
        dbg_str(EV_DETAIL, "add fd =%d into io map", fd);
        io_map->add(io_map, event->ev_fd, event);

        eb->trustee_io(eb,event);
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
        eb->reclaim_io(eb,event);
        timer->del(timer, event);

        //del fd in map
        ret = io_map->del(io_map, fd);
        if (ret < 0) {
            dbg_str(EV_WARNNING,"not found fd in io_map,ret=%d",ret);
        } else {
            dbg_str(EV_WARNNING,"del fd =%d in io_map", fd);
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

    dbg_str(EV_DETAIL,"event base active io event, fd = %d", fd);

    ret = io_map->search(io_map, fd, (void **)&event);
    dbg_str(EV_DETAIL,"search ret=%d",ret);

    if (ret < 0) {
        dbg_str(EV_WARNNING,"not found fd = %d in io_map,ret=%d",fd, ret);
    } else {
        dbg_str(EV_DETAIL, "event addr:%p, ev_callback=%p",
                event, event->ev_callback);

        event->ev_callback(event->ev_fd, 0, event->ev_arg);

        if (event->ev_events & EV_PERSIST) {
            dbg_str(EV_DETAIL, "persist event, readd io");
            timer->del(timer, event);
            event->ev_timeout = event->ev_tv;
            timer->add(timer, event);
        } else {
            dbg_str(EV_DETAIL,"del event");
            eb->del(eb, event);
        } 
    }

    return 0;
}

static int __activate_signal_old(Event_Base *eb, int fd, short events)
{
    Map *map = eb->evsig.map;
    event_t *event = NULL;
    char buf[16];
    char *p = NULL;
    int ret;

    dbg_str(EV_DETAIL,"event base active signal event, signal = %d, ncount=%d", fd, events);

    map->search(map, &fd, (void **)&event);
    if (event != NULL) {
        dbg_str(EV_WARNNING,"activate_signal, find signal=%d", fd);
        event->ev_callback(event->ev_fd, 0, event->ev_arg);
    } else {
        dbg_str(EV_WARNNING,"activate_signal, get event addr error");
        return -1;
    }

    return 0;
}

static void __signal_list_for_each_callback(void *element)
{
    event_t *event = (event_t *)element;
    event->ev_callback(event->ev_fd, 0, event->ev_arg);
}

static int __activate_signal(Event_Base *eb, int fd, short events)
{
    Map *map = eb->evsig.map;
    List *list = eb->evsig.list;
    event_t *event = NULL;
    char buf[16];
    char *p = NULL;
    int ret;

    dbg_str(EV_DETAIL,"event base active signal event, signal = %d, ncount=%d", fd, events);

    list->remove_all(list);

    map->search_all_same_key(map, fd, list);
    if (list->count(list) != 0) {
        dbg_str(EV_WARNNING,"activate_signal, list count=%d", list->count(list));
        dbg_str(EV_WARNNING,"activate_signal, find signal=%d", fd);
        list->for_each(list, __signal_list_for_each_callback);
    } else {
        dbg_str(EV_WARNNING,"activate_signal, get event addr error");
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
    if (event != NULL) {
        timeval_now(&now, NULL);
        if (timeval_cmp(&now, &event->ev_timeout) >= 0) {
            dbg_str(EV_DETAIL,"process_timeout, event addr:%p",event);
            event->ev_callback(event->ev_fd, 0, event->ev_arg);
            if (event->ev_events & EV_PERSIST) {
                timer->remove(timer, event);
                event->ev_timeout = event->ev_tv;
                timer->add(timer, event);
            } else {
                timer->del(timer, event);
            }
        }
    }
}

static int __loop(Event_Base *eb)
{
    Timer *timer = eb->timer;
    struct timeval tv, *tv_p;

    set_break_signal(eb);

    while (eb->break_flag == 0) {
        tv_p = &tv;
        timer->timeout_next(timer, &tv_p);
        eb->dispatch(eb, tv_p);
        __process_timeout_events(eb);
    }

    dbg_str(EV_DETAIL, "break Event_Base loop");
}

static class_info_entry_t event_base_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","loop",__loop,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","activate_io",__activate_io,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","activate_signal",__activate_signal,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","add",__add,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","del",__del,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","trustee_io",NULL,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","reclaim_io",NULL,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","dispatch",NULL,sizeof(void *)},
    [13] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Event_Base", event_base_class_info);

void test_obj_eb()
{
    Event_Base *eb;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(EV_SUC, "configurator_t addr:%p",c);
    cfg_config(c, "/Event_Base", CJSON_STRING, "name", "alan eb") ;  

    eb = OBJECT_NEW(allocator, Event_Base,c->buf);

    object_dump(eb, "Event_Base", buf, 2048);
    dbg_str(EV_DETAIL,"Event_Base dump: %s",buf);

    object_destroy(eb);
    cfg_destroy(c);
}
