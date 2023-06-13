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
#include <unistd.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Linked_List.h>
#include <libobject/concurrent/Producer.h>
#include <libobject/concurrent/api.h>

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

static int __add_worker(Producer *producer, void *worker)
{
    Worker *w            = (Worker *)worker;
    Event_Thread *thread = &producer->parent;
    List *workers_list   = producer->workers;

    while (producer->parent.flags < EVTHREAD_STATE_RUNNING){
        dbg_str(DBG_SUC, "default_producer not ready, waiting...");
        usleep(100000);
    }

    dbg_str(DBG_SUC, "producer %p add worker, worker:%p, fd:%d",
            producer, worker, w->event.ev_fd);

    w->producer = producer;
    thread->add_event(thread, (void *)&w->event);
    workers_list->add(workers_list, worker);

    return 0;
}

static int __del_worker(Producer *producer, void *worker)
{
    Worker *w            = (Worker *)worker;
    Event_Thread *thread = &producer->parent;
    List *workers_list   = producer->workers;

    while (producer->parent.flags < EVTHREAD_STATE_RUNNING){
        dbg_str(DBG_SUC, "default_producer not ready, waiting...");
        usleep(100000);
    }

    dbg_str(DBG_DETAIL, "producer %p del worker, worker:%p, fd:%d",
            producer, worker, w->event.ev_fd);

    w->producer = producer;
    thread->del_event(thread, (void *)&w->event);
    workers_list->remove_element(workers_list, worker);
}

static int __add_dispatcher(Producer *producer, void *worker)
{
}

static int __del_dispatcher(Producer *producer, void *worker)
{
}

static int __close(Producer *producer)
{
    Event_Thread *thread = &producer->parent;

    thread->stop(thread);

    return 0;
}

static class_info_entry_t producer_class_info[] = {
    Init_Obj___Entry(0, Event_Thread, parent),
    Init_Nfunc_Entry(1, Producer, construct, __construct),
    Init_Nfunc_Entry(2, Producer, deconstruct, __deconstrcut),
    Init_Nfunc_Entry(3, Producer, add_worker, __add_worker),
    Init_Nfunc_Entry(4, Producer, del_worker, __del_worker),
    Init_Vfunc_Entry(5, Producer, add_dispatcher, __add_dispatcher),
    Init_Vfunc_Entry(6, Producer, del_dispatcher, __del_dispatcher),
    Init_Vfunc_Entry(7, Producer, start, NULL),
    Init_Vfunc_Entry(8, Producer, close, __close),
    Init_End___Entry(9, Producer),
};
REGISTER_CLASS("Producer", producer_class_info);

Producer *producer_get_default_instance()
{
    return global_default_producer;
}

int producer_init_default_instance(char *thread_service, char *signal_service)
{
    Producer *producer;
    allocator_t *allocator = allocator_get_default_instance();
    configurator_t * c;

    c = cfg_alloc(allocator); 
    cfg_config_str(c, "/Event_Thread", "thread_service", thread_service); 
    cfg_config_str(c, "/Event_Thread", "signal_service", signal_service); 

    // dbg_str(DBG_DETAIL, "Producer init str:%s", c->buf);
    dbg_str(DBG_VIP, "producer_init_default_instance");
    producer = OBJECT_NEW(allocator, Producer, c->buf);
    global_default_producer = producer;

    producer->start(producer);
    cfg_destroy(c);

    return 0;
}

int producer_destroy_default_instance()
{
    Producer *producer = producer_get_default_instance();

    dbg_str(DBG_VIP, "producer_destroy_default_instance start");
    while (producer->parent.flags < EVTHREAD_STATE_RUNNING){
        dbg_str(DBG_SUC, "default_producer not ready, waiting...");
        usleep(100000);
    }
    producer->close(producer);
    while (producer->parent.flags != EVTHREAD_STATE_DESTROYED) usleep(1000000);

    object_destroy(producer);
    dbg_str(DBG_VIP, "producer_destroy_default_instance end");

    return 0;
}
