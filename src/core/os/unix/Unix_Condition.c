/**
 * @file Unix_Condition.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-10-07
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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Thread.h>
#include <libobject/core/Linked_Queue.h>
#include "Unix_Condition.h"

static int __construct(Unix_Condition *cond, char *init_str)
{
    pthread_cond_init(&cond->cond,NULL);
    pthread_mutex_init(&cond->mutex, NULL);

    return 0;
}

static int __deconstrcut(Unix_Condition *cond)
{
    pthread_cond_destroy(&cond->cond);   
    pthread_mutex_destroy(&cond->mutex);

    return 0;
}

static int __wait(Unix_Condition *cond)
{     
    return pthread_cond_wait(&cond->cond, &cond->mutex);
}

static int __signal(Unix_Condition *cond)
{
    return pthread_cond_signal(&cond->cond);
}

static int __broadcast(Unix_Condition *cond)
{
    return pthread_cond_broadcast(&cond->cond);
}

static class_info_entry_t unix_cond_class_info[] = {
    Init_Obj___Entry(0, Condition, parent),
    Init_Nfunc_Entry(1, Unix_Condition, construct, __construct),
    Init_Nfunc_Entry(2, Unix_Condition, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3, Unix_Condition, wait, __wait),
    Init_Vfunc_Entry(4, Unix_Condition, signal, __signal),
    Init_Vfunc_Entry(5, Unix_Condition, broadcast, __broadcast),
    Init_End___Entry(6, Unix_Condition),
};
REGISTER_CLASS("Unix_Condition", unix_cond_class_info);


/*
static Unix_Condition  *cond;
static Mutex_Lock   *mutex_lock;
static Linked_Queue *queue;


pthread_cond_t cond;
pthread_mutex_t mutex;

static void * producer(void *arg)
{
    sleep(2);
    int buf[6]={1,2,3,4,5,56};
    int i;

    Thread * t1 = (Thread *)arg;
    t1->detach(t1);
    //mutex_lock->lock(mutex_lock);
#if 0
    pthread_mutex_lock(&mutex);
    for( i = 0 ; i < sizeof(buf)/sizeof(int); i++ ) {
        queue->add(queue,&buf[i]);
    }

    pthread_cond_signal(&cond);
    dbg_str(DBG_IMPORTANT,"producer notify");
    pthread_mutex_unlock(&mutex);
#else
    mutex_lock->lock(mutex_lock);
    for( i = 0 ; i < sizeof(buf)/sizeof(int); i++ ) {
        queue->add(queue,&buf[i]);
    }

    cond->signal(cond);
    dbg_str(DBG_IMPORTANT,"producer notify");
    mutex_lock->unlock(mutex_lock);

#endif 

    return NULL;
}

static void * consumer(void *arg)
{

    void *element;
    Thread * t1 = (Thread *)arg;
    t1->detach(t1);


#if 0

    pthread_mutex_lock(&mutex);
    //queue->is_empty(queue);
    while(queue->is_empty(queue)) {
        pthread_cond_wait(&cond,&mutex);
    }
    queue->peek_back(queue,&element);
    pthread_mutex_unlock(&mutex);

#else

    mutex_lock->lock(mutex_lock);
    while (queue->is_empty(queue)) {

        cond->wait(cond);
    }

    queue->peek_back(queue,&element);

    mutex_lock->unlock(mutex_lock);
#endif 

    dbg_str(DBG_IMPORTANT,"element value:%d ",*(int *)element);
    queue->reset(queue);

    return NULL;
}


static int test_cond_produce_consume()
{


    allocator_t *allocator = allocator_get_default_alloc();

    cond  = OBJECT_NEW(allocator,Unix_Condition,NULL);
    mutex_lock = OBJECT_NEW(allocator,Mutex_Lock,NULL);
    queue      = OBJECT_NEW(allocator,Linked_Queue,NULL);

    cond->setlock(cond,mutex_lock);

    dbg_str(DBG_ERROR,"Unix_Condition addr: %p",cond);
    dbg_str(DBG_ERROR,"Mutex_Lock addr: %p",mutex_lock);
    dbg_str(DBG_ERROR,"queue addr: %p",queue);


    Thread * t1 = OBJECT_NEW(allocator,Thread,NULL);
    t1->set_run_routine(t1,producer);
    t1->start(t1);


    Thread * t2 = OBJECT_NEW(allocator,Thread,NULL);
    t2->set_run_routine(t2,consumer);
    t2->start(t2);


    object_destroy(cond);
    object_destroy(mutex_lock);
    object_destroy(queue);

    return 1;
}


REGISTER_STANDALONE_TEST_FUNC(test_cond_produce_consume);
*/
