/**
 * @file Condition.c
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
#include <libobject/core/os/Condition.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/Thread.h>
#include <libobject/core/Linked_Queue.h>

static int __construct(Condition *condition, char *init_str)
{
    allocator_t *allocator = condition->obj.allocator;
    int ret;

    ret = pthread_cond_init(&condition->cond,NULL);
    condition->lock = NULL;
    dbg_str(DBG_DETAIL, "condition construct, condition addr:%p ret:%d", condition,ret );

    return ret;
}

static int __deconstrcut(Condition *condition)
{
    dbg_str(DBG_DETAIL, "condition deconstruct, condition addr:%p", condition);
    int ret;
    void *tret;
    ret = pthread_cond_destroy(&condition->cond);   
    return ret;
}

static int __setlock(Condition *condition, Mutex_Lock *lock)
{
   
    condition->lock = lock;
    //dbg_str(DBG_IMPORTANT,"Mutex_Lock addr: %p",condition->lock);
    return 0; 
}

static int __wait(Condition *condition)
{     
     return pthread_cond_wait(&(condition->cond),&(condition->lock->mutex));
}

static int __signal(Condition *condition)
{
    return pthread_cond_signal(&condition->cond);
}

static int __broadcast(Condition *condition)
{
     return pthread_cond_broadcast(&condition->cond);
}

static class_info_entry_t condition_class_info[] = {
    Init_Obj___Entry(0, Obj, obj),
    Init_Nfunc_Entry(1, Condition, construct, __construct),
    Init_Nfunc_Entry(2, Condition, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3, Condition, setlock, __setlock),
    Init_Vfunc_Entry(4, Condition, wait, __wait),
    Init_Vfunc_Entry(5, Condition, signal, __signal),
    Init_Vfunc_Entry(6, Condition, broadcast, __broadcast),
    Init_End___Entry(7, Condition),
};
REGISTER_CLASS("Condition", condition_class_info);


 static Condition    * condition  ;
 static Mutex_Lock   * mutex_lock  ;
 static Linked_Queue * queue      ;


pthread_cond_t cond ;
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
    
    condition->signal(condition);
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

          condition->wait(condition);
     }

     queue->peek_back(queue,&element);

     mutex_lock->unlock(mutex_lock);
     #endif 

    dbg_str(DBG_IMPORTANT,"element value:%d ",*(int *)element);
    queue->reset(queue);
    
    return NULL;
}


static int test_condition_produce_consume()
{

    
    allocator_t *allocator = allocator_get_default_alloc();

    condition  = OBJECT_NEW(allocator,Condition,NULL);
    mutex_lock = OBJECT_NEW(allocator,Mutex_Lock,NULL);
    queue      = OBJECT_NEW(allocator,Linked_Queue,NULL);

    condition->setlock(condition,mutex_lock);

    dbg_str(DBG_ERROR,"Condition addr: %p",condition);
    dbg_str(DBG_ERROR,"Mutex_Lock addr: %p",mutex_lock);
    dbg_str(DBG_ERROR,"queue addr: %p",queue);
    

    Thread * t1 = OBJECT_NEW(allocator,Thread,NULL);
    t1->set_run_routine(t1,producer);
    t1->start(t1);


    Thread * t2 = OBJECT_NEW(allocator,Thread,NULL);
    t2->set_run_routine(t2,consumer);
    t2->start(t2);


    object_destroy(condition);
    object_destroy(mutex_lock);
    object_destroy(queue);
    
    return 1;
}


REGISTER_STANDALONE_TEST_FUNC(test_condition_produce_consume);
