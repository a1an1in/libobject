/**
 * @file producer.c
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
#include <libobject/utils/dbg/debug.h>
#include <libobject/utils/config/config.h>
#include <libobject/utils/timeval/timeval.h>
#include <libobject/concurrent/producer.h>
#include <libobject/concurrent/worker.h>

Producer *global_default_producer;

static int __construct(Producer *producer,char *init_str)
{
    /*
     *allocator_t *allocator = producer->obj.allocator;
     */
    configurator_t * c;
    char buf[2048];

    dbg_str(EV_DETAIL,"producer construct, producer addr:%p",producer);

    return 0;
}

static int __deconstrcut(Producer *producer)
{
    dbg_str(EV_DETAIL,"producer deconstruct,producer addr:%p",producer);

    return 0;
}

static int __set(Producer *producer, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        producer->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        producer->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        producer->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        producer->deconstruct = value;
    }
    else if (strcmp(attrib, "add_worker") == 0) {
        producer->add_worker = value;
    } else if (strcmp(attrib, "del_worker") == 0) {
        producer->del_worker = value;
    } else if (strcmp(attrib, "add_dispatcher") == 0) {
        producer->add_dispatcher = value;
    } else if (strcmp(attrib, "del_dispatcher") == 0) {
        producer->del_dispatcher = value;
    }
    else if (strcmp(attrib, "start") == 0) {
        producer->start = value;
    } 
    else {
        dbg_str(EV_DETAIL,"producer set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Producer *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"producer get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __add_worker(Producer *producer, void *worker)
{
    Worker *w = (Worker *)worker;
    Event_Thread *thread = &producer->parent;

    w->producer = producer;
    thread->add_event(thread, (void *)&w->event);

    return 0;
}

static int __del_worker(Producer *producer, void *worker)
{
}

static int __add_dispatcher(Producer *producer, void *worker)
{
}

static int __del_dispatcher(Producer *producer, void *worker)
{
}

static class_info_entry_t producer_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Event_Thread","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","add_worker",__add_worker,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","del_worker",__del_worker,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","add_dispatcher",__add_dispatcher,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","del_dispatcher",__del_dispatcher,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "start", NULL, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Producer",producer_class_info);

Producer *global_get_default_producer()
{
    return global_default_producer;
}

__attribute__((constructor(ATTRIB_PRIORITY_CONCURRENT))) void
default_producer_constructor()
{
    Producer *producer;
    allocator_t *allocator = allocator_get_default_alloc();

    producer = OBJECT_NEW(allocator, Producer, NULL);
    global_default_producer = producer;

    producer->start(producer);
}

__attribute__((destructor(ATTRIB_PRIORITY_CONCURRENT))) void
default_producer_destructor()
{
    Producer *producer = global_get_default_producer();
    object_destroy(producer);
}

void test_obj_producer()
{
    Producer *producer;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(EV_SUC, "configurator_t addr:%p",c);
    cfg_config(c, "/Producer", CJSON_STRING, "name", "alan producer") ;  

    producer = OBJECT_NEW(allocator, Producer,c->buf);

    object_dump(producer, "Producer", buf, 2048);
    dbg_str(EV_DETAIL,"Producer dump: %s",buf);

    object_destroy(producer);
    cfg_destroy(c);
}
