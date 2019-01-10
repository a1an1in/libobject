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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/event_base.h>
#include <libobject/event/select_base.h>
#include <libobject/core/linked_queue.h>
#include <libobject/net/socket/unix_udp_socket.h>
#include <libobject/event/event_thread.h>
#include <libobject/libobject.h>

static int __construct(Event_Thread *thread, char *init_str)
{
    allocator_t *allocator = thread->parent.obj.allocator;
    Event_Base *eb;

    dbg_str(EV_DETAIL,"Event_Thread construct, thread addr:%p",thread);

    thread->eb       = (Event_Base *)OBJECT_NEW(allocator, Select_Base, NULL);
    thread->ev_queue = (Queue *)OBJECT_NEW(allocator, Linked_Queue, NULL);
    thread->flags    = 0;

    return 0;
}

static int __deconstrcut(Event_Thread *thread)
{
    int ret = 0;

    dbg_str(EV_DETAIL,"Event thread deconstruct,thread addr:%p",thread);
    object_destroy(thread->s);
    object_destroy(thread->c);
    object_destroy(thread->ev_queue);
    object_destroy(thread->eb);

    return ret;
}

static int __set(Event_Thread *thread, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        thread->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        thread->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        thread->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        thread->deconstruct = value;
    } else if (strcmp(attrib, "add_event") == 0) {
        thread->add_event = value;
    } else if (strcmp(attrib, "del_event") == 0) {
        thread->del_event = value;
    } /*inherited methods*/
    else if (strcmp(attrib, "start") == 0) {
        thread->start = value;
    } else if (strcmp(attrib, "set_start_routine") == 0) {
        thread->set_start_routine = value;
    } else if (strcmp(attrib, "set_start_arg") == 0) {
        thread->set_start_arg = value;
    } /*vitural methods*/
    else if (strcmp(attrib, "start_routine") == 0) {
        thread->start_routine = value;
    } else if (strcmp(attrib, "detach") == 0) {
        thread->detach = value;
    }
    else {
        dbg_str(EV_DETAIL,"thread set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Thread *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"thread get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }

    return NULL;
}

static int __add_event(Event_Thread *thread, void *event)
{
    Socket *c = thread->c;

    dbg_str(EV_DETAIL,"add event, c socket:%p", c);

    if (event != NULL) {
        thread->ev_queue->add(thread->ev_queue, event);
    } else {
        return -1;
    }

    if (c->write(c, "a", 1) != 1) {
        dbg_str(EV_ERROR,"ctl_write error");
        return -1;
    }

    return 0;
}

static int __del_event(Event_Thread *thread, void *event)
{
    Socket *c = thread->c;

    dbg_str(EV_DETAIL,"del event");

    if (event != NULL) {
        thread->ev_queue->add(thread->ev_queue, event);
    } else {
        return -1;
    }

    if (c->write(c, "d", 1) != 1) {
        dbg_str(EV_ERROR,"ctl_write error");
        return -1;
    }

    return 0;
}

static void event_thread_server_socket_ev_callback(int fd, short events, void *arg)
{
    Event_Thread *event_thread = (Event_Thread *)arg;
    Queue *ev_queue            = event_thread->ev_queue;
    Event_Base *eb             = event_thread->eb;
    Socket *s                  = event_thread->s;
    event_t *event;
    char buf[1];
    int len;

    if (s->read(s, buf, 1) != 1) {
        dbg_str(EV_WARNNING,"ctl_read error");
        return ;
    }

    switch(buf[0]) {
        case 'a': 
            {
                dbg_str(EV_SUC,"rcv add event command, event=%p", event);
                ev_queue->remove(ev_queue, (void **)&event);
                eb->add(eb, event);
                break;
            }
        case 'd': 
            {
                dbg_str(DBG_SUC,"rcv del event command, event=%p", event);
                ev_queue->remove(ev_queue, (void **)&event);
                eb->del(eb, event);
                break;
            }
        default:
            break;
    }

    return;
}

static void *__start_routine(void *arg)
{
    Event_Thread *et       = (Event_Thread *)arg;
    Thread * tt            = et;
    allocator_t *allocator = et->parent.obj.allocator;
    Event_Base *eb         = et->eb;
    event_t *event         = &et->server_socket_event;
    int fds[2]             = {0};
    Socket *s, *c;
    char *libobject_run_path;
    char buf[2048];
    char server_addr[100];
    static int count;

    count++;
    libobject_run_path = libobject_get_run_path();
    sprintf(server_addr, "%s/%s_%d", libobject_run_path, 
            "event_thread_unix_socket_server", count); 
    dbg_str(EV_IMPORTANT,"Event_Thread, start_routine:%p, server_addr:%s",
            arg, server_addr);
            
    tt->detach(tt);
    s = OBJECT_NEW(allocator, Unix_Udp_Socket, NULL);
    s->bind(s, server_addr, NULL); 

    c = OBJECT_NEW(allocator, Unix_Udp_Socket, NULL);
    c->connect(c, server_addr, NULL);

    et->s = s;
    et->c = c;

    event->ev_fd              = et->s->fd;
    event->ev_events          = EV_READ | EV_PERSIST;
    event->ev_timeout.tv_sec  = 0;
    event->ev_timeout.tv_usec = 0;
    event->ev_callback        = event_thread_server_socket_ev_callback;
    event->ev_arg             = arg;
    eb->add(eb, event);

    et->flags = EVTHREAD_STATE_RUNNING; //?????the event may havn't been added to evbase

    eb->loop(eb);

    et->flags = EVTHREAD_STATE_DESTROYED;
    dbg_str(EV_IMPORTANT,"Event Thread, out start routine");
}

static class_info_entry_t event_thread_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Thread", "parent", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "add_event", __add_event, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_FUNC_POINTER, "", "del_event", __del_event, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "start", NULL, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "set_start_routine", NULL, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "set_start_arg", NULL, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "start_routine", __start_routine, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "detach", NULL, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Event_Thread",event_thread_class_info);



static void
test_timeout_cb(int fd, short event, void *arg)
{
    struct timeval newtime, difference;
    double elapsed;
    static struct timeval lasttime;

    gettimeofday(&newtime, NULL);
    timeval_sub(&newtime, &lasttime, &difference);

    elapsed  = difference.tv_sec + (difference.tv_usec / 1.0e6);
    lasttime = newtime;

    dbg_str(EV_SUC,"timeout_cb called at %d: %.3f seconds elapsed.",
            (int)newtime.tv_sec, elapsed);
}

void test_obj_event_thread()
{
    Event_Thread *thread;
    allocator_t *allocator = allocator_get_default_alloc();
    event_t event;
    char buf[2048];

    thread = OBJECT_NEW(allocator, Event_Thread, NULL);

    object_dump(thread, "Thread", buf, 2048);
    dbg_str(EV_DETAIL,"Thread dump: %s",buf);
    /*
     *thread->set_start_routine(thread, event_thread_start_routine);
     *thread->set_start_arg(thread, thread);
     */
    thread->start(thread);
    
    sleep(1);
    event.ev_fd              = -1;
    event.ev_events          = EV_READ | EV_PERSIST;
    event.ev_timeout.tv_sec  = 2;
    event.ev_timeout.tv_usec = 0;
    event.ev_callback        = test_timeout_cb;
    event.ev_arg             = &event;
    thread->add_event(thread, (void *)&event);

    sleep(10);
    pause();

    object_destroy(thread);
}


