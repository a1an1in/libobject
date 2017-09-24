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
#include <libobject/utils/dbg/debug.h>
#include <libobject/event/event_base.h>
#include <libobject/utils/config/config.h>
#include <libobject/utils/timeval/timeval.h>

static int __construct(Event_Base *eb,char *init_str)
{
    allocator_t *allocator = eb->obj.allocator;
    configurator_t * c;
    char buf[2048];
    /*
     *uint8_t key_size = 4;
     *uint16_t value_size = 4;
     *uint16_t bucket_size = 20;
     */

    dbg_str(EV_DETAIL,"eb construct, eb addr:%p",eb);

    eb->break_flag = 0;

    eb->timer = OBJECT_NEW(allocator, Rbtree_Timer,NULL);

    c = cfg_alloc(allocator); 
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_size", "4") ;  
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "value_size", "8") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "bucket_size", "20") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_type", "1");
    eb->io_map    = OBJECT_NEW(allocator, Hash_Map, c->buf);

    eb->map_iter = OBJECT_NEW(allocator, Hmap_Iterator,NULL);

    dbg_str(EV_DETAIL,"base addr:%p, io_map addr :%p, map_iter:%p, timer:%p",
            eb, eb->io_map, eb->map_iter, eb->timer);


    cfg_destroy(c);

    return 0;
}

static int __deconstrcut(Event_Base *eb)
{
    dbg_str(EV_DETAIL,"eb deconstruct,eb addr:%p",eb);

    //release evsig
    object_destroy(eb->timer);
    object_destroy(eb->map_iter);
    object_destroy(eb->io_map);

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
    } else if (strcmp(attrib, "active_io") == 0) {
        eb->active_io = value;
    } else if (strcmp(attrib, "active_signal") == 0) {
        eb->active_signal = value;
    }
    else if (strcmp(attrib, "construct") == 0) {
        eb->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        eb->deconstruct = value;
    } else if (strcmp(attrib, "add_io") == 0) {
        eb->add_io = value;
    } else if (strcmp(attrib, "del_io") == 0) {
        eb->del_io = value;
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
    char buffer[16] = {0};

    dbg_str(EV_DETAIL,"base addr:%p, io_map addr :%p, map_iter:%p, timer:%p, event:%p",
            eb, eb->io_map, eb->map_iter, eb->timer, event);

    if (event->ev_events & EV_SIGNAL) {
        evsig_add(eb, event);
    } else {
        event->ev_tv = event->ev_timeout;
        addr_to_buffer(event,buffer);
        dbg_buf(EV_DETAIL,"buffer:", buffer, 4);
        io_map->insert(io_map, &fd, buffer);

        eb->add_io(eb,event);
        timer->add(timer, event);

    }
    return (0);
}

static int __del(Event_Base *eb, event_t *event) 
{
    Timer *timer   = eb->timer;
    int fd         = event->ev_fd;
    Iterator *iter = eb->map_iter;
    Map *io_map    = eb->io_map;
    int ret;

    if (event->ev_events & EV_SIGNAL) {
    } else {
        eb->del_io(eb,event);
        timer->del(timer, event);

        //del fd in map
        ret = io_map->search(io_map, &fd, iter);
        if (ret < 0) {
            dbg_str(DBG_WARNNING,"not found fd in io_map,ret=%d",ret);
        } else {
            ret = io_map->del(io_map, iter);
            dbg_str(DBG_WARNNING,"del fd =%d in io_map", fd);
        }
    }

    return 0;
}

static int __active_io(Event_Base *eb, int fd, short events)
{
    Timer *timer   = eb->timer;
    Map *io_map    = eb->io_map;
    Iterator *iter = eb->map_iter;
    struct timeval tv;
    char *p;
    char buf[255];
    int len;
    event_t *event;
    int ret;

    dbg_str(EV_DETAIL,"event base active io event, fd = %d", fd);

    ret = io_map->search(io_map, &fd, iter);
    dbg_str(DBG_WARNNING,"search ret=%d",ret);

    if (ret < 0) {
        dbg_str(DBG_WARNNING,"not found fd in io_map,ret=%d",ret);
    } else {
        p = iter->get_vpointer(iter);
        dbg_buf(EV_DETAIL,"buffer:", p, 4);
        event = (event_t *)buffer_to_addr(p);
        dbg_str(EV_DETAIL,"event addr:%p, ev_callback=%p", event, event->ev_callback);
        event->ev_callback(event->ev_fd, 0, event->ev_arg);

        if (event->ev_events & EV_PERSIST) {
            dbg_str(EV_DETAIL,"persist event, readd io");
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

static int __active_signal(Event_Base *eb, int fd, short events)
{
    rbtree_map_t *sig_map = eb->evsig.sig_map;
    rbtree_map_pos_t it;
    event_t *event;
    char buf[16];
    char *p = NULL;

    dbg_str(EV_DETAIL,"event base active signal event, signal = %d, ncount=%d", fd, events);

    rbtree_map_search_by_numeric_key(sig_map, fd,&it);
    if (it.rb_node_p != NULL) {
        p = rbtree_map_pos_get_pointer(&it);
        event = (event_t *)buffer_to_addr(p);

        if (event != NULL) {
            dbg_str(EV_DETAIL,"event=%p, ev_callback=%p", event, event->ev_callback);
            event->ev_callback(event->ev_fd, 0, event->ev_arg);
        } else {
            dbg_str(DBG_WARNNING,"active_signal, get event addr error");
            return -1;
        }
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
            event->ev_callback(event->ev_fd, 0, event);
            if (event->ev_events & EV_PERSIST) {
                timer->del(timer, event);
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
    struct timeval tv, *tv_p = &tv;

    set_break_signal(eb);

    while(eb->break_flag == 0) {
        timer->timeout_next(timer, &tv_p);
        eb->dispatch(eb, tv_p);
        __process_timeout_events(eb);
    }

    dbg_str(EV_WARNNING, "break Event_Base loop");
}

static class_info_entry_t event_base_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","loop",__loop,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","active_io",__active_io,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","active_signal",__active_signal,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","add",__add,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_FUNC_POINTER,"","del",__del,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","add_io",NULL,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","del_io",NULL,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","dispatch",NULL,sizeof(void *)},
    [13] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Event_Base",event_base_class_info);

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


