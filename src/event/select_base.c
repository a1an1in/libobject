/**
 * @file select_base.c
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
#include <libobject/event/select_base.h>
#include <libobject/utils/config/config.h>

static int __construct(Select_Base *eb,char *init_str)
{
    dbg_str(OBJ_DETAIL,"select constructor");

    eb->maxfdp = 1;
    FD_ZERO(&eb->event_readset_in);
    FD_ZERO(&eb->event_writeset_in);
    FD_ZERO(&eb->event_readset_out);
    FD_ZERO(&eb->event_writeset_out);

    return 0;
}

static int __deconstrcut(Select_Base *eb)
{
    dbg_str(OBJ_DETAIL,"select constructor");

    return 0;
}

static int __set(Select_Base *eb, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        eb->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        eb->get = value;
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
    } 
    else if (strcmp(attrib, "active_io") == 0) {
        eb->active_io = value;
    }
    else {
        dbg_str(OBJ_DETAIL,"eb set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Select_Base *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(OBJ_WARNNING,"eb get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __add(Select_Base *b, event_t *e)
{
    int fd = e->ev_fd;
    Event_Base *p = (Event_Base *)b;
    Timer *timer  = p->timer;

    /*
     *if (fd < 0) {
     *    dbg_str(DBG_WARNNING,"not add this fd");
     *    return 0;
     *}
     */

    b->maxfdp = b->maxfdp -1 > fd ? b->maxfdp : fd + 1; 

    dbg_str(DBG_SUC,"select base add event, fd =%d, maxfdp=%d", fd, b->maxfdp);

    if (e->ev_events & EV_READ)
        FD_SET(fd, &b->event_readset_in);
    if (e->ev_events & EV_WRITE)
        FD_SET(fd, &b->event_writeset_in);

    //add for test>>
    b->event_readset_out  = b->event_readset_in;
    b->event_writeset_out = b->event_writeset_in;
    //<<
    
    timer->add(timer, e);

    return (0);
}

static int __del(Select_Base *b, event_t *e) 
{
    int fd = e->ev_fd;
    unsigned short events = e->ev_events;

    dbg_str(DBG_SUC,"select base add event");

    //.. maxfdp

    if (events & EV_READ)
        FD_CLR(fd, &b->event_readset_in);

    if (events & EV_WRITE)
        FD_CLR(fd, &b->event_writeset_in);

    return 0;
}

static int __dispatch(Select_Base *b, struct timeval *tv)
{
    int res=0, i, j, nfds = b->maxfdp;

    b->event_readset_out  = b->event_readset_in;
    b->event_writeset_out = b->event_writeset_in;

    res = select(nfds, &b->event_readset_out,
            &b->event_writeset_out, NULL, tv);

    if (res == -1) {
        dbg_str(DBG_WARNNING,"select error");
        return (0);
    } else{
        dbg_str(DBG_SUC,"select base dispatch event, res=%d, tv=%d",res, tv->tv_sec);
    }

    i = random() % nfds;
    for (j = 0; j < nfds; ++j) {
        if (++i >= nfds)
            i = 0;
        res = 0;

        if (FD_ISSET(i, &b->event_readset_out))
            res |= EV_READ;
        if (FD_ISSET(i, &b->event_writeset_out))
            res |= EV_WRITE;

        if (res == 0)
            continue;

        dbg_str(DBG_SUC,"fd %d has event", i);
        b->active_io((Event_Base *)b,i, res);
    }

    return 0;
}

static class_info_entry_t select_base_class_info[] = {
    [0] = {ENTRY_TYPE_OBJ,"Event_Base","base",NULL,sizeof(void *)},
    [1] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5] = {ENTRY_TYPE_FUNC_POINTER,"","add",__add,sizeof(void *)},
    [6] = {ENTRY_TYPE_FUNC_POINTER,"","del",__del,sizeof(void *)},
    [7] = {ENTRY_TYPE_FUNC_POINTER,"","dispatch",__dispatch,sizeof(void *)},
    [8] = {ENTRY_TYPE_IFUNC_POINTER,"","active_io",NULL,sizeof(void *)},
    [9] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Select_Base",select_base_class_info);

static void test_ev_callback(int fd, short events, void *arg)
{
    dbg_str(DBG_SUC,"hello world, event");
}

void test_obj_select_base()
{
    Event_Base *eb;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    char buf[2048];
    cjson_t *root, *e, *s;
    event_t event;

    eb = OBJECT_NEW(allocator, Select_Base, NULL);

    dbg_str(DBG_DETAIL,"run at here, eb=%p", eb);

    object_dump(eb, "Select_Base", buf, 2048);
    dbg_str(DBG_DETAIL,"Select_Base dump: %s",buf);

    event.ev_timeout.tv_sec  = 2;
    event.ev_timeout.tv_usec = 0;
    event.ev_fd = -1;
    event.ev_callback = test_ev_callback;

    eb->add(eb, &event);
    eb->loop(eb);

    pause();
    object_destroy(eb);
}


