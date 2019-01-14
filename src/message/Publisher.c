/**
 * @file Sender.c
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
#include <libobject/message/publisher.h> 
#include <libobject/message/centor.h>
#include <libobject/message/subscriber.h>

static int __construct(Publisher *publisher, char *init_str)
{
    allocator_t *allocator = publisher->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL, "publisher construct, publisher addr:%p", publisher);

    return 0;
}

static int __deconstrcut(Publisher *publisher)
{
    dbg_str(DBG_DETAIL, "publisher deconstruct, publisher addr:%p", publisher);

    return 0;
}

static int __set(Publisher *publisher, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        publisher->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        publisher->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        publisher->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        publisher->deconstruct = value;
    } else if (strcmp(attrib, "connect_centor") == 0) {
        publisher->connect_centor = value;
    } else if (strcmp(attrib, "publish") == 0) {
        publisher->publish = value;
    } else if (strcmp(attrib, "publish_message") == 0) {
        publisher->publish_message = value;
    } 
    else {
        dbg_str(DBG_DETAIL, "publisher set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Publisher *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARNNING, "publisher get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __connect_centor(Publisher *publish, Centor *centor)
{
    dbg_str(DBG_DETAIL, "publish message");
    publish->centor = centor;
}

static int __publish(Publisher *publish, message_t *message)
{
    dbg_str(DBG_DETAIL, "publish message, unsurported now");
}

int 
__publish_message(Publisher *publisher, char *what, void *opaque)
{
    message_t * message;
    Centor *centor = (Centor *)publisher->centor;
    allocator_t *allocator = publisher->obj.allocator;

    dbg_str(DBG_IMPORTANT, "publish raw message, publisher %p, what %s, opaque %p", publisher, what, opaque);

    message = message_alloc(allocator);
    message_set(message, "what", what);
    message_set(message, "opaque", opaque);
    message_set(message, "publisher", publisher);

    centor->message_queue->add(centor->message_queue, message);
    centor->c->write(centor->c, "p", 1);
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "connect_centor", __connect_centor, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "publish", __publish, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "publish_message", __publish_message, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Publisher", concurent_class_info);

static void test_on_pause(message_t *message, void *arg)
{
    Subscriber *subscriber = (Subscriber *)arg;
    dbg_str(DBG_SUC, "subscriber receive a message:%s", (char *)message->what);
    dbg_str(DBG_DETAIL, "message arg:%p", arg);
}

static void test_xxxx(message_t *message, void *arg)
{
    Subscriber *subscriber = (Subscriber *)arg;
    dbg_str(DBG_SUC, "subscriber receive a message:%s", (char *)message->what);
    dbg_str(DBG_DETAIL, "message arg:%p", arg);
}

static int test_message_publisher()
{
    Centor *centor;
    Publisher *publisher;
    Subscriber *subscriber;
    allocator_t *allocator = allocator_get_default_alloc();
    char * test_str = "on_pause";

    char *test_xxxx = "test_xxxx";

    char *test_android = "test_android";

    centor     = OBJECT_NEW(allocator, Centor, NULL);
    publisher  = OBJECT_NEW(allocator, Publisher, NULL);
    subscriber = OBJECT_NEW(allocator, Subscriber, NULL);

    subscriber->connect_centor(subscriber, centor);
    subscriber->add_method(subscriber, "on_pause", test_on_pause, allocator);
    subscriber->subscribe(subscriber, publisher);

 
    subscriber->add_method(subscriber, "test_xxxx", test_xxxx, allocator);   
    //subscriber->subscribe(subscriber, publisher);
    

    subscriber->add_method(subscriber, "test_android", test_android, allocator);   
    //subscriber->subscribe(subscriber, publisher);

    //subscriber->subscribe(subscriber, publisher);

    dbg_str(DBG_SUC, "%p subscribe a publisher, publisher addr:%p", subscriber, publisher);
    dbg_str(DBG_SUC, "message handler arg:%p", allocator);

    publisher->connect_centor(publisher, centor);
    publisher->publish_message(publisher, test_str, strlen(test_str));
    
    publisher->connect_centor(publisher, centor);
    publisher->publish_message(publisher, test_str, strlen(test_str));

    pause();
    object_destroy(centor);
    object_destroy(subscriber);
    object_destroy(publisher);

    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_message_publisher);
