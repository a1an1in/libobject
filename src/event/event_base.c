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

static int timeval_cmp(struct timeval *k1,struct timeval *k2)
{
    if (    k1->tv_sec > k2->tv_sec || 
            (k1->tv_sec == k2->tv_sec && k1->tv_usec > k2->tv_usec))
    {
        return 1;
    } else if (k1->tv_sec == k2->tv_sec && k1->tv_usec == k2->tv_usec) {
        return 0;
    } else {
        return -1;
    }
}

static int timeval_now(struct timeval *t, struct timezone *tz)
{
    gettimeofday(t, tz);
}

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

    eb->timer = OBJECT_NEW(allocator, Rbtree_Timer,NULL);

    c = cfg_alloc(allocator); 
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_size", "4") ;  
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "value_size", "8") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "bucket_size", "20") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_type", "1");
    eb->map    = OBJECT_NEW(allocator, Hash_Map, c->buf);

    eb->map_iter = OBJECT_NEW(allocator, Hmap_Iterator,NULL);

    dbg_str(EV_DETAIL,"base addr:%p, map addr :%p, map_iter:%p, timer:%p",
            eb, eb->map, eb->map_iter, eb->timer);


    memset(c->buf, 0, c->buf_len);
    cfg_config(c, "/List", CJSON_NUMBER, "value_size", "8") ;  

    eb->list      = OBJECT_NEW(allocator, Linked_List, c->buf);
    eb->list_iter = OBJECT_NEW(allocator, LList_Iterator,NULL);

    object_dump(eb->list, "Linked_List", buf, 2048);
    dbg_str(DBG_DETAIL,"List dump: %s",buf);

    cfg_destroy(c);

    return 0;
}

static int __deconstrcut(Event_Base *eb)
{
    dbg_str(EV_DETAIL,"eb deconstruct,eb addr:%p",eb);

    object_destroy(eb->timer);
    object_destroy(eb->map_iter);
    object_destroy(eb->map);
    object_destroy(eb->list);
    object_destroy(eb->list_iter);

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

static int __add(Event_Base *b, event_t *e)
{
    Timer *timer  = b->timer;
    Map *map = b->map;
    int fd = e->ev_fd;
    char buffer[16] = {0};

    dbg_str(EV_DETAIL,"base addr:%p, map addr :%p, map_iter:%p, timer:%p, event:%p",
            b, b->map, b->map_iter, b->timer, e);
    addr_to_buffer(e,buffer);
    dbg_buf(DBG_DETAIL,"buffer:", buffer, 4);
    map->insert(map, &fd, buffer);

    b->add_io(b,e);
    timer->add(timer, e);

    return (0);
}

static int __del(Event_Base *b, event_t *e) 
{
    Timer *timer  = b->timer;

    b->del_io(b,e);
    timer->del(timer, e);

    return 0;
}

static int __active_io(Event_Base *b, int fd, short events)
{
    struct timeval tv;
    event_t *event;
    char *p;
    char buf[255];
    int len;
    Map *map = b->map;
    Iterator *iter = b->map_iter;
    int ret;

    dbg_str(EV_SUC,"event base active io event, fd = %d", fd);

    ret = map->search(map, &fd, iter);
    dbg_str(DBG_WARNNING,"search ret=%d",ret);

    if (ret < 0) {
        dbg_str(DBG_WARNNING,"not found fd in map,ret=%d",ret);
    } else {
        p = iter->get_vpointer(iter);
        dbg_buf(DBG_DETAIL,"buffer:", p, 4);
        event = (event_t *)buffer_to_addr(p);
        dbg_str(DBG_SUC,"event addr:%p", event);
        event->ev_callback(fd, 0, NULL);

        b->add(b, event);
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
            event->ev_callback(event->ev_fd, 0, NULL);
            timer->del(timer, event);
        }
    }
}

static int __process_active_events(Event_Base *eb)
{
    Timer *timer = eb->timer;
    struct timeval now, search;
    event_t *event;

    dbg_str(DBG_SUC,"process_active_events");
}


static int __loop(Event_Base *eb)
{
    Timer *timer = eb->timer;
    struct timeval tv, *tv_p = &tv;
    int count = 10;

    while(count--) {
        timer->timeout_next(timer, &tv_p);
        eb->dispatch(eb, tv_p);
        __process_timeout_events(eb);
        __process_active_events(eb);
    }

    dbg_str(EV_WARNNING, "break Event_Base loop");
}

static class_info_entry_t event_base_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","loop",__loop,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","active_io",__active_io,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","add",__add,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_FUNC_POINTER,"","del",__del,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","add_io",NULL,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","del_io",NULL,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","dispatch",NULL,sizeof(void *)},
    [12] = {ENTRY_TYPE_END},
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

