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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/linked_list.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/concurrent/producer.h>
#include <libobject/concurrent/worker.h>

Producer *global_default_producer;

static int __construct(Producer *producer, char *init_str)
{
	Obj *obj = (Obj *)producer;
    allocator_t *allocator = obj->allocator;

    dbg_str(CONCURRENT_DETAIL, "producer construct, producer addr:%p", producer);
    producer->workers = OBJECT_NEW(allocator, Linked_List, NULL);

    return 0;
}

static int __deconstrcut(Producer *producer)
{
    dbg_str(CONCURRENT_DETAIL, "producer deconstruct, producer addr:%p", producer);
    object_destroy(producer->workers);

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
        dbg_str(CONCURRENT_DETAIL, "producer set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Producer *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(CONCURRENT_WARNNING, "producer get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __add_worker(Producer *producer, void *worker)
{
    Worker *w            = (Worker *)worker;
    Event_Thread *thread = &producer->parent;
    List *workers_list   = producer->workers;

    w->producer = producer;
    thread->add_event(thread, (void *)&w->event);
    /*
     *workers_list->add(workers_list, worker);
     */

    return 0;
}

static int __del_worker(Producer *producer, void *worker)
{
    Worker *w            = (Worker *)worker;
    Event_Thread *thread = &producer->parent;
    List *workers_list   = producer->workers;

    dbg_str(DBG_SUC, "producer del worker");
    w->producer = producer;
    thread->del_event(thread, (void *)&w->event);
    /*
     *workers_list->remove_element(workers_list, worker);
     */
}

static int __add_dispatcher(Producer *producer, void *worker)
{
}

static int __del_dispatcher(Producer *producer, void *worker)
{
}

static class_info_entry_t producer_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Event_Thread", "parent", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_worker", __add_worker, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "del_worker", __del_worker, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add_dispatcher", __add_dispatcher, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "del_dispatcher", __del_dispatcher, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "start", NULL, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Producer", producer_class_info);

Producer *global_get_default_producer()
{
    Producer *producer = global_default_producer;

    while (producer->parent.flags < EVTHREAD_STATE_RUNNING) sleep(1);
    return producer;
}

int default_producer_constructor()
{
    Producer *producer;
    allocator_t *allocator = allocator_get_default_alloc();

    ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY=%d, run default producer\n", 
            REGISTRY_CTOR_PRIORITY_CONCURRENT);
    producer = OBJECT_NEW(allocator, Producer, NULL);
    global_default_producer = producer;

    producer->start(producer);

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_CONCURRENT, 
                   default_producer_constructor);

int default_producer_destructor()
{
    Producer *producer = global_get_default_producer();

    ATTRIB_PRINT("REGISTRY_DTOR_PRIORITY=%d, destruct default producer\n", 
            REGISTRY_DTOR_PRIORITY_CONCURRENT);
    while (producer->parent.flags != EVTHREAD_STATE_DESTROYED) sleep(1);

    object_destroy(producer);
}
REGISTER_DTOR_FUNC(REGISTRY_DTOR_PRIORITY_CONCURRENT, 
                   default_producer_destructor);



void test_obj_producer()
{
    Producer *producer;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(CONCURRENT_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/Producer", CJSON_STRING, "name", "alan producer") ;  

    producer = OBJECT_NEW(allocator, Producer, c->buf);

    object_dump(producer, "Producer", buf, 2048);
    dbg_str(CONCURRENT_DETAIL, "Producer dump: %s", buf);

    object_destroy(producer);
    cfg_destroy(c);
}
