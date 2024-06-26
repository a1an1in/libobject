/**
 * @file consumer.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-09-24
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
#include <libobject/core/utils/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/Consumer.h>

static int __construct(Consumer *consumer, char *init_str)
{
    allocator_t *allocator = consumer->obj.allocator;

    dbg_str(EV_DETAIL, "consumer construct, consumer addr:%p", consumer);

    return 0;
}

static int __deconstrcut(Consumer *consumer)
{
    dbg_str(EV_DETAIL, "consumer deconstruct, consumer addr:%p", consumer);

    return 0;
}

static int __set(Consumer *consumer, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        consumer->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        consumer->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        consumer->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        consumer->deconstruct = value;
    } 
    else {
        dbg_str(EV_DETAIL, "consumer set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Consumer *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARN, "consumer get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t consumer_class_info[] = {
    Init_Obj___Entry(0, Obj, obj),
    Init_End___Entry(1, Consumer),
};
REGISTER_CLASS(Consumer, consumer_class_info);

void test_obj_consumer()
{
    Consumer *consumer;
    allocator_t *allocator = allocator_get_default_instance();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(EV_SUC, "configurator_t addr:%p", c);
    cfg_config_str(c, "/Consumer", "name", "alan consumer") ;  

    consumer = OBJECT_NEW(allocator, Consumer, c->buf);

    object_dump(consumer, "Consumer", buf, 2048);
    dbg_str(EV_DETAIL, "Consumer dump: %s", buf);

    object_destroy(consumer);
    cfg_destroy(c);
}
