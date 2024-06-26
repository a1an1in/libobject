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
#include <libobject/concurrent/event/Event_Base.h>
#include <libobject/concurrent/event/Select_Base.h>
#include <libobject/core/Linked_Queue.h>
#include <libobject/concurrent/event/Event_Thread.h>
#include <libobject/config.h>

static int __construct(Event_Thread *thread, char *init_str)
{
    allocator_t *allocator = thread->parent.obj.allocator;
    Event_Base *eb;

    dbg_str(EV_DETAIL,"Event_Thread construct, thread addr:%p",thread);
    dbg_str(DBG_DETAIL,"Event_Thread construct, thread service:%s, signal service:%s",
            STR2A(thread->thread_service), STR2A(thread->signal_service));

    eb = (Event_Base *)object_new(allocator, "Select_Base", NULL);
    eb->set(eb, "/Event_Base/signal_service", STR2A(thread->signal_service));
    eb->init(eb);
    thread->eb = eb;
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
    object_destroy(thread->thread_service);
    object_destroy(thread->signal_service);
    dbg_str(EV_VIP,"Event thread destroy server sokcet");
    object_destroy(thread->s);
    dbg_str(EV_VIP,"Event thread destroy client sokcet");
    object_destroy(thread->c);
    object_destroy(thread->eb);
#if (defined(WINDOWS_USER_MODE))
    dbg_str(DBG_INFO,"windows WSACleanup");
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
    dbg_str(EV_VIP, "event_thread, add_event fd=%d", event->ev_fd);

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

    dbg_str(EV_VIP, "event_thread, del_event fd=%d", event->ev_fd);
    eb->del(eb, event);

    /* let the option take effect, not only update map, fresh the base
     * (take select for example, update read and write set fd also) 
     */
    if (c->send(c, "d", 1, 0) != 1) {
        return -1;
    }

    return 0;
}

static void event_thread_notifier_callback(int fd, short events, void *arg)
{
    Event_Thread *event_thread = (Event_Thread *)arg;
    Socket *s = event_thread->s;
    char buf[1];
    int len, ret;

    if ((ret = s->recv(s, buf, 1, 0)) != 1) {
        dbg_str(EV_WARN,"event thread notifier was waked up, fd:%d ret:%d!", fd, ret);
        return ;
    }
    dbg_str(EV_VIP, "event_thread notifier received message:%c", buf[0]);
    switch(buf[0]) {
        case 'a': 
        case 'd': 
            break;
        case 'e': //exit
            dbg_str(DBG_SUC, "event_thread received exit message!");
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

    dbg_str(DBG_INFO, "Event_Thread, start_routine:%p, service:%s",
            arg, STR2A(et->thread_service));
            
    tt->detach(tt);
    s = object_new(allocator, "Inet_Udp_Socket", NULL);
    s->bind(s, "127.0.0.1", STR2A(et->thread_service));
    s->setnonblocking(s);
    et->s = s;

    c = object_new(allocator, "Inet_Udp_Socket", NULL);
    c->connect(c, "127.0.0.1", STR2A(et->thread_service));
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
    dbg_str(EV_VIP,"Event Thread, out start routine");
}

static int __stop(Event_Thread *thread)
{
    Socket *c      = thread->c;
    Event_Base *eb = thread->eb;

    eb->break_flag = 1;

    if (c->send(c, "e", 1, 0) != 1) {//to make option task effect
        return -1;
    }

    dbg_str(EV_VIP,"Event Thread has been stopped!");

    return 0;
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
    Init_Vfunc_Entry(9 , Event_Thread, stop, __stop),
    Init_Str___Entry(10, Event_Thread, thread_service, NULL),
    Init_Str___Entry(11, Event_Thread, signal_service, NULL),
    Init_End___Entry(12, Event_Thread),
};
REGISTER_CLASS(Event_Thread, event_thread_class_info);