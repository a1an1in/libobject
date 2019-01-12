
/**
 * @file Subscriber.c
 * @Synopsis  
 * @author alan lin
 * @version 1
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
#include <libobject/message/subscriber.h> 
#include <libobject/message/centor.h>
#include <libobject/message/message.h>
#include <libobject/core/rbtree_map.h>

static void message_handler(void *arg)
{
    Subscriber *subscriber = (Subscriber *)arg;
    message_method_t *method = NULL;

    message_t *message = (message_t *)subscriber->message;

    dbg_str(DBG_DETAIL, "message handler opaque:%p, msg.what=%s",
            subscriber->opaque,
            message->what);

    subscriber->method_map->search(subscriber->method_map, message->what, (void **)&method);
    if (method != NULL) {
        method->func(message, method->arg);
    } else {
        dbg_str(DBG_DETAIL, "not found method, key=%s", message->what);
    }

}

static  void release_registered_method(void *key, void *element, void *arg)
{
    allocator_t *allocator = (allocator_t *)arg;
    allocator_mem_free(allocator, element);
}

static int __construct(Subscriber *subscriber, char *init_str)
{
    allocator_t *allocator = subscriber->obj.allocator;
    configurator_t * config;

    dbg_str(EV_DETAIL, "subscriber construct, subscriber addr:%p", subscriber);

    subscriber->message_handler = message_handler;

    subscriber->method_map = OBJECT_NEW(allocator, RBTree_Map, NULL);
    subscriber->method_map->set_cmp_func(subscriber->method_map, 
                                         string_key_cmp_func);

    return 0;
}

static int __deconstrcut(Subscriber *subscriber)
{
    allocator_t *allocator = subscriber->obj.allocator;

    dbg_str(EV_DETAIL, "subscriber deconstruct, subscriber addr:%p", subscriber);
    subscriber->method_map->for_each_arg(subscriber->method_map,
                                         release_registered_method,
                                         allocator);
    object_destroy(subscriber->method_map);

    return 0;
}

static int __set(Subscriber *subscriber, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        subscriber->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        subscriber->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        subscriber->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        subscriber->deconstruct = value;
    } else if (strcmp(attrib, "connect_centor") == 0) {
        subscriber->connect_centor = value;
    } else if (strcmp(attrib, "subscribe") == 0) {
        subscriber->subscribe = value;
    } else if (strcmp(attrib, "add_method") == 0) {
        subscriber->add_method = value;
    } 
    else {
        dbg_str(EV_DETAIL, "subscriber set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Subscriber *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING, "subscriber get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __connect_centor(Subscriber *subscriber, void *centor)
{
    dbg_str(DBG_DETAIL, "subscriber connect centor");
    subscriber->centor = centor;
}

static void __printf_map(void *key, void *element)
{
    char **publisher  = (char **)key;
    dbg_str(DBG_IMPORTANT, "subscriber map, key=%p", *publisher);
}

static int __subscribe(Subscriber *subscriber, void *publisher)
{
    Centor *centor = (Centor *)subscriber->centor;
    dbg_str(DBG_DETAIL, "subscriber %p subscribe publisher %p", subscriber, publisher);

    subscriber->publisher = publisher;
    centor->subscriber_map->add(centor->subscriber_map, subscriber, publisher);

    centor->subscriber_map->for_each(centor->subscriber_map, __printf_map);
    return 0;
}

static int
__add_method(Subscriber *subscriber,char *method_name,
             void (*func)(message_t *, void *), void *arg)
{
    allocator_t *allocator = subscriber->obj.allocator;

    message_method_t *method;

    method = allocator_mem_alloc(allocator, sizeof(message_method_t));

    method->func = func;
    method->arg = arg;
    
    return subscriber->method_map->add(subscriber->method_map, method_name, method);
}

static class_info_entry_t concurent_class_info[] = {
    [0] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5] = {ENTRY_TYPE_VFUNC_POINTER, "", "connect_centor", __connect_centor, sizeof(void *)}, 
    [6] = {ENTRY_TYPE_VFUNC_POINTER, "", "subscribe", __subscribe, sizeof(void *)}, 
    [7] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_method", __add_method, sizeof(void *)}, 
    [8] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Subscriber", concurent_class_info);

