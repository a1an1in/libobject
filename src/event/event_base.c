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

    dbg_str(EV_DETAIL,"eb construct, eb addr:%p",eb);

    eb->timer = OBJECT_NEW(allocator, Rbtree_Timer,NULL);

    return 0;
}

static int __deconstrcut(Event_Base *eb)
{
    dbg_str(EV_DETAIL,"eb deconstruct,eb addr:%p",eb);

    object_destroy(eb->timer);

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
    } else if (strcmp(attrib, "active_io") == 0) {
        eb->active_io = value;
    }
    else if (strcmp(attrib, "construct") == 0) {
        eb->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        eb->deconstruct = value;
    } else if (strcmp(attrib, "add") == 0) {
        eb->add = value;
    } else if (strcmp(attrib, "del") == 0) {
        eb->del = value;
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

static int __active_io(Event_Base *eb, int fd, short events)
{
    struct timeval tv;
    event_t event;
    char buf[255];
    int len;

    dbg_str(EV_DETAIL,"event base active io event, fd = %d", fd);

    len = read(fd, buf, sizeof(buf) - 1);

    if (len == -1) {
        perror("read");
        return;
    } else if (len == 0) {
        fprintf(stderr, "Connection closed\n");
        return;
    }

    buf[len] = '\0';
    fprintf(stdout, "Read: %s\n", buf);

    /*
     *event.ev_fd = fd;
     *event.ev_events = EV_READ;
     *eb->add(eb, &event);
     */

    return 0;
}

static int __timeout_process(Event_Base *eb)
{
    Timer *timer = eb->timer;
    struct timeval now, search;
    event_t *event;

    event = timer->first(timer);
    if (event != NULL) {
        timeval_now(&now, NULL);
        if (timeval_cmp(&now, &event->ev_timeout) >= 0) {
            dbg_str(EV_DETAIL,"timeout_process, event addr:%p",event);
            event->ev_callback(event->ev_fd, 0, NULL);
            timer->del(timer, event);
        }
    }
}

static int __loop(Event_Base *eb)
{
    Timer *timer = eb->timer;
    struct timeval tv, *tv_p = &tv;
    int count = 10;

    while(count--) {
        timer->timeout_next(timer, &tv_p);
        eb->dispatch(eb, tv_p);
        __timeout_process(eb);
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
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","add",NULL,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","del",NULL,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","dispatch",NULL,sizeof(void *)},
    [10] = {ENTRY_TYPE_END},
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


