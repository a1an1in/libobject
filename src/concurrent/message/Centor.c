
/**
 * @file Centor.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2018-10-28
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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/message/Centor.h> 
#include <libobject/concurrent/message/message.h> 
#include <libobject/concurrent/message/Subscriber.h>
#include <libobject/concurrent/message/Publisher.h>
#include <libobject/concurrent/worker_api.h>
#include <libobject/core/Linked_Queue.h>
#include <libobject/core/Rbtree_Map.h>
#include <libobject/config.h>

/* message 是基于concurrent 实现的，所以跟concurrent的使用范围一样，
 * 只能用于进程中用作模块间通信*/

static void
message_centor_ev_callback(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    Centor *centor = (Centor *)worker->opaque;
    Socket *s = centor->s;
    char buf[1];

    if (s->recv(s, buf, 1, 0) != 1) {
        dbg_str(DBG_WARN, "message centor read ev error, centor:%p", centor);
        return ;
    }

    switch(buf[0]) {
        case 'p': {
                dbg_str(DBG_DETAIL, "a publisher pub a message");
                break;
            }
        default:
            dbg_str(DBG_WARN, "read a unsupported message");
            break;
    }

    worker->work_callback(worker);

    return;
}

/*
 * find the subscribers who care the message
 * */
static void map_for_each_arg_callback(void *key, void *element, void *arg)
{
    Subscriber *subscriber = (Subscriber *)key;
    Publisher *publisher =(Publisher *)element;

    message_t *message = (message_t *)arg;
     
    if (publisher == message->publisher) {
        dbg_str(DBG_DETAIL, "publisher %p publish a message which has a subscriber: %p", 
                key, subscriber);
        subscriber->message = message;
        subscriber->message_handler(subscriber);
    } else {
        dbg_str(DBG_WARN, 
                "publisher %p publish a message, publisher in map %p ", 
                message->publisher, key);
    }
}

static void message_centor_work_callback(void *task)
{
    Worker *worker = (Worker *)task;
    Centor *centor = (Centor *)worker->opaque;
    message_t *message;

    dbg_str(DBG_DETAIL, "centor worker callback");
    dbg_str(DBG_DETAIL, "centor addr:%p", worker->opaque);

    centor->message_queue->remove(centor->message_queue, 
                                  (void **)&message);

    dbg_str(DBG_DETAIL, "message addr %p, publisher addr:%p",
            message, message->publisher);

    centor->subscriber_map->for_each_arg(centor->subscriber_map, 
                                         map_for_each_arg_callback, message);

    message_destroy(message);
}

static int __construct(Centor *centor, char *init_str)
{
    allocator_t *allocator = centor->obj.allocator;
    Socket *s, *c;
    char service[10] = {0};

    dbg_str(DBG_VIP, "centor construct, centor addr:%p", centor);
    sprintf(service, "%d", MESSAGE_SERVICE);

    s = object_new(allocator, "Inet_Udp_Socket", NULL);
    s->bind(s, "127.0.0.1", service);
    s->setnonblocking(s);
    centor->s = s;

    c = object_new(allocator, "Inet_Udp_Socket", NULL);
    c->connect(c, "127.0.0.1", service);
    c->setnonblocking(c);
    centor->c = c;

    dbg_str(DBG_VIP, "centor add io_worker fd=%d", centor->s->fd);
    centor->worker = io_worker(allocator,
                               centor->s->fd,
                               NULL, NULL,
                               message_centor_ev_callback,
                               message_centor_work_callback,
                               centor);

    centor->message_queue  = object_new(allocator, "Linked_Queue", NULL);
    centor->subscriber_map = object_new(allocator, "RBTree_Map", NULL);
    dbg_str(DBG_VIP, "centor construct, worker addr:%p", centor->worker);

    return 0;
}

static int __deconstrcut(Centor *centor)
{
    dbg_str(DBG_DETAIL, "centor deconstruct, centor addr:%p, worker:%p", centor, centor->worker);
    worker_destroy(centor->worker);
    object_destroy(centor->s);
    object_destroy(centor->c);
    object_destroy(centor->message_queue);
    object_destroy(centor->subscriber_map);

    return 0;
}

static class_info_entry_t concurent_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Centor, construct, __construct),
    Init_Nfunc_Entry(2 , Centor, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Centor, set, NULL),
    Init_Vfunc_Entry(4 , Centor, get, NULL),
    Init_End___Entry(5 , Centor),
};
REGISTER_CLASS(Centor, concurent_class_info);

