/**
 * @thread Thread.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-27
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
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/Event_Base.h>
#include <libobject/event/Select_Base.h>
#include <libobject/core/Linked_Queue.h>
#include <libobject/event/Event_Thread.h>
#include <libobject/libobject.h>
#include <libobject/config.h>

static int __construct(Event_Thread *thread, char *init_str)
{
    allocator_t *allocator = thread->parent.obj.allocator;
    Event_Base *eb;

    dbg_str(EV_DETAIL,"Event_Thread construct, thread addr:%p",thread);

    thread->eb    = (Event_Base *)object_new(allocator, "Select_Base", NULL);
    thread->flags = 0;

    return 0;
}

static int __deconstrcut(Event_Thread *thread)
{
    int ret          = 0;
    Event_Thread *et = (Event_Thread *)thread;
    Event_Base *eb   = et->eb;
    event_t *event   = &et->notifier_event;

    dbg_str(EV_DETAIL,"Event thread deconstruct,thread addr:%p",thread);

    thread->del_event(thread, event);
    object_destroy(thread->s);
    object_destroy(thread->c);
    object_destroy(thread->eb);
#if (defined(WINDOWS_USER_MODE))
    printf("windows WSACleanup\n"); 
    WSACleanup();
#endif
    
    return ret;
}

static int __add_event(Event_Thread *thread, event_t *event)
{
    Socket *c = thread->c;
    Event_Base *eb = thread->eb;

    if (event == NULL) {
        return -1;
    }

    eb->add(eb, event);

    if (c->send(c, "a", 1, 0) != 1) {//to make option task effect
        return -1;
    }

    return 0;
}

static int __del_event(Event_Thread *thread, event_t *event)
{
    Socket *c      = thread->c;
    Event_Base *eb = thread->eb;

    if (event == NULL) {
        return -1;
    }

    eb->del(eb, event);

    if (c->send(c, "d", 1, 0) != 1) {//to make option task effect
        return -1;
    }

    return 0;
}

static void event_thread_notifier_callback(int fd, short events, void *arg)
{
    Event_Thread *event_thread = (Event_Thread *)arg;
    Socket *s = event_thread->s;
    char buf[1];
    int len;

    if (s->recv(s, buf, 1, 0) != 1) {
        dbg_str(EV_WARNNING,"ctl_read error");
        return ;
    }

    switch(buf[0]) {
        case 'a': 
        case 'd': 
            break;
        default:
            break;
    }

    return;
}

static void *__start_routine(void *arg)
{
    Event_Thread *et       = (Event_Thread *)arg;
    Thread * tt            = (Thread *)et;
    allocator_t *allocator = et->parent.obj.allocator;
    Event_Base *eb         = et->eb;
    event_t *event         = &et->notifier_event;
    Socket *s, *c;
    char service[10] = {0};

    sprintf(service, "%d", EVENT_THREAD_SERVICE);

    dbg_str(EV_IMPORTANT,"Event_Thread, start_routine:%p, service:%s",
            arg, service);
            
    tt->detach(tt);
    s = object_new(allocator, "Inet_Udp_Socket", NULL);
    s->bind(s, "127.0.0.1", service);
    s->setnonblocking(s);
    et->s = s;

    c = object_new(allocator, "Inet_Udp_Socket", NULL);
    c->connect(c, "127.0.0.1", service);
    c->setnonblocking(c);
    et->c = c;

    event->ev_fd              = et->s->fd;
    event->ev_events          = EV_READ | EV_PERSIST;
    event->ev_timeout.tv_sec  = 0;
    event->ev_timeout.tv_usec = 0;
    event->ev_callback        = event_thread_notifier_callback;
    event->ev_arg             = arg;
    eb->add(eb, event);

    et->flags = EVTHREAD_STATE_RUNNING; //?????the event may havn't been added to evbase

    eb->loop(eb);

    et->flags = EVTHREAD_STATE_DESTROYED;
    dbg_str(EV_IMPORTANT,"Event Thread, out start routine");
}

static class_info_entry_t event_thread_class_info[] = {
    Init_Obj___Entry(0 , Thread, parent),
    Init_Nfunc_Entry(1 , Event_Thread, construct, __construct),
    Init_Nfunc_Entry(2 , Event_Thread, deconstruct, __deconstrcut),
    Init_Nfunc_Entry(3 , Event_Thread, add_event, __add_event),
    Init_Nfunc_Entry(4 , Event_Thread, del_event, __del_event),
    Init_Vfunc_Entry(5 , Event_Thread, start, NULL),
    Init_Vfunc_Entry(6 , Event_Thread, set_start_routine, NULL),
    Init_Vfunc_Entry(7 , Event_Thread, set_start_arg, NULL),
    Init_Vfunc_Entry(8 , Event_Thread, start_routine, __start_routine),
    Init_Vfunc_Entry(9 , Event_Thread, detach, NULL),
    Init_End___Entry(10, Event_Thread),
};
REGISTER_CLASS("Event_Thread",event_thread_class_info);

