
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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/message/centor.h> 
#include <libobject/message/message.h> 
#include <libobject/message/subscriber.h>
#include <libobject/message/publisher.h>
#include <libobject/net/socket/unix_udp_socket.h>
#include <libobject/core/linked_queue.h>
#include <libobject/core/rbtree_map.h>
#include <libobject/libobject.h>

#define DEFAULT_CENTOR_UNIX_SERVER_PATH "/tmp/default_centor_unix_socket_path"

static void
message_centor_ev_callback(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    char buf[1];

    if (read(fd, buf, 1) != 1) {
        dbg_str(DBG_WARNNING, "message centor read ev error");
        return ;
    }

    switch(buf[0]) {
        case 'p': {
                dbg_str(DBG_SUC, "a publisher pub a message");
                break;
            }
        default:
            dbg_str(DBG_WARNNING, "read a unsupported message");
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
    Publisher *publisher    =(Publisher *)element;

    message_t *message     = (message_t *)(arg);

    if ( message == NULL ) { 
        return;
    } 

    if (publisher == message->publisher) {
        dbg_str(DBG_SUC, "publisher %p publish a message which has a subscriber: %p", 
                publisher,subscriber);
        subscriber->message = message;
        subscriber->message_handler(subscriber);
    } else {
        dbg_str(DBG_WARNNING, 
                "publisher %p publish a message, publisher in map %p ", 
                (message)->publisher, key);
    }
}

static void message_centor_work_callback(void *task)
{
    Worker *worker = (Worker *)task;
    Centor *centor = (Centor *)worker->opaque;
    message_t *message;

    dbg_str(DBG_SUC, "centor worker callback");
    dbg_str(DBG_DETAIL, "centor addr:%p", worker->opaque);
    centor->message_queue->remove(centor->message_queue, 
                                  (void **)&message);
    dbg_str(DBG_DETAIL, "message addr %p, publisher addr:%p message queue size:%d ", message, message->publisher,centor->message_queue->size(centor->message_queue));
    centor->subscriber_map->for_each_arg(centor->subscriber_map, 
                                          map_for_each_arg_callback, message);

    message_destroy(message);
}

static int __construct(Centor *centor, char *init_str)
{
    allocator_t *allocator = centor->obj.allocator;
    Socket *s, *c;
    configurator_t * config;
    char server_addr[100];
    char *libobject_run_path;
    static int count;

    dbg_str(DBG_SUC, "centor construct, centor addr:%p", centor);

    count++;
    libobject_run_path = libobject_get_run_path();
    sprintf(server_addr, "%s/%s_%d", libobject_run_path, 
            "message_unix_socket_server", count); 

    s = OBJECT_NEW(allocator, Unix_Udp_Socket, NULL);
    s->bind(s, server_addr, NULL); 

    c = OBJECT_NEW(allocator, Unix_Udp_Socket, NULL);
    c->connect(c, server_addr, NULL);

    centor->s = s;
    centor->c = c;

    dbg_str(DBG_SUC, "centor add io_worker fd=%d", centor->s->fd);
    centor->worker = io_worker(allocator, centor->s->fd, 
                               NULL, message_centor_ev_callback, 
                               message_centor_work_callback,
                               NULL, centor);

    centor->message_queue  = OBJECT_NEW(allocator, Linked_Queue, NULL);
    centor->subscriber_map = OBJECT_NEW(allocator, RBTree_Map, NULL);

    return 0;
}

static int __deconstrcut(Centor *centor)
{
    dbg_str(DBG_DETAIL, "centor deconstruct, centor addr:%p", centor);
    io_worker_destroy(centor->worker);
    object_destroy(centor->s);
    object_destroy(centor->c);
    object_destroy(centor->message_queue);
    object_destroy(centor->subscriber_map);

    return 0;
}

static int __set(Centor *centor, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        centor->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        centor->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        centor->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        centor->deconstruct = value;
    } 
    else {
        dbg_str(DBG_DETAIL, "centor set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Centor *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARNNING, "centor get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Centor", concurent_class_info);

int test_obj_message_centor()
{
    Centor *centor;
    allocator_t *allocator = allocator_get_default_alloc();
    char * test_str = "p";

    centor = OBJECT_NEW(allocator, Centor, NULL);

    centor->c->write(centor->c, test_str, 1);
    pause();
    object_destroy(centor);

    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_obj_message_centor);
