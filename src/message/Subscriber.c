
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

static int __construct(Subscriber *subscriber, char *init_str)
{
    allocator_t *allocator = subscriber->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(EV_DETAIL, "subscriber construct, subscriber addr:%p", subscriber);

    return 0;
}

static int __deconstrcut(Subscriber *subscriber)
{
    dbg_str(EV_DETAIL, "subscriber deconstruct, subscriber addr:%p", subscriber);

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
    } else if (strcmp(attrib, "add_message_handler") == 0) {
        subscriber->add_message_handler = value;
    } else if (strcmp(attrib, "add_message_handler_arg") == 0) {
        subscriber->add_message_handler_arg = value;
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

static int __subscribe(Subscriber *subscriber, void *publisher)
{
    Centor *centor = (Centor *)subscriber->centor;
    dbg_str(DBG_DETAIL, "subscriber subscribe publisher");

    subscriber->publisher = publisher;
    centor->subscriber_map->add(centor->subscriber_map, &publisher, subscriber);

    return 0;
}

static int __add_message_handler(Subscriber *subscriber, void (*func)(void *))
{
    subscriber->message_handler = func;
    return 0;
}

static int __add_message_handler_arg(Subscriber *subscriber, void *arg)
{
    subscriber->message_handler_arg = arg;
    return 0;
}

static class_info_entry_t concurent_class_info[] = {
    [0] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5] = {ENTRY_TYPE_VFUNC_POINTER, "", "connect_centor", __connect_centor, sizeof(void *)}, 
    [6] = {ENTRY_TYPE_VFUNC_POINTER, "", "subscribe", __subscribe, sizeof(void *)}, 
    [7] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_message_handler", __add_message_handler, sizeof(void *)}, 
    [8] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_message_handler_arg", __add_message_handler_arg, sizeof(void *)}, 
    [9] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Subscriber", concurent_class_info);
