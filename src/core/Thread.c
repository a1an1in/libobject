/**
 * @thread Thread.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-27
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
#include <libobject/core/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/Thread.h>

static int __construct(Thread *thread, char *init_str)
{
    allocator_t *allocator = thread->obj.allocator;
    configurator_t * c;
    char buf[2048];
    thread->is_run = 0;
    thread->arg = NULL;
    thread->joinable = 1;
    thread->tid      = -1;
    dbg_str(DBG_IMPORTANT, "Thread construct, thread addr:%p", thread);

    return 0;
}

static int __deconstrcut(Thread *thread)
{
    int ret;
    void *tret;

    dbg_str(DBG_IMPORTANT, "thread deconstruct, thread addr:%p", thread);

    dbg_str(DBG_IMPORTANT, "thread deconstruct, out");
    if (thread->tid !=0) {
        /*
         *ret = pthread_join(thread->tid, &tret);
         *if (ret != 0) {
         *    dbg_str(OBJ_WARNNING, "can't join with thread tid=%d", thread->tid);
         *}
         */
    }

    dbg_str(DBG_IMPORTANT, "thread deconstruct, out");

    return 0;
}

static int __start(Thread *thread)
{
    void *arg;
    if (thread->is_run) {
        return -1;
    }
    if (thread->start_routine == NULL) {
        return -1;
    }

    if (thread->arg == NULL) {
        thread->arg = thread;
    }
    if ((pthread_create(&thread->tid, NULL, thread->start_routine, thread->arg)) != 0) {
        dbg_str(OBJ_WARNNING, "pthread start error");
    }

    thread->is_run = 0;

    return 0;
}

static int __set_start_routine(Thread *thread, void *routine)
{
   
    thread->start_routine = routine;
    return 0;
}

static int __set_start_arg(Thread *thread, void *arg)
{
    thread->arg = arg;
}

static int __set_opaque(Thread *thread, void *arg)
{
    thread->opaque = arg;
}

static void *__start_routine(void *arg)
{
    dbg_str(OBJ_DETAIL, "start_routine");
    Thread * thread = (Thread *)arg;
    if ( thread == NULL ){
        return;
    }
    thread->is_run = 1; 

    while (1) {
        usleep(100000);

        while ( thread->is_run ) {
            thread->execute(arg);
        }
        dbg_str(DBG_WARNNING,"current thread finished");
    }
    return NULL;
}


static void __stop(Thread * thread)
{
    if ( thread->is_run ) {
        thread->is_run = 0;
    }
}

static int __get_status(Thread *thread)
{
    return thread->is_run;
}

static int __set_run_routine(Thread *thread,void *func )
{
    if ( thread == NULL || func == NULL ) {
        return -1;
    }
    thread->execute = func;
    return 0;
}

static void __run(Thread *thread)
{
    if ( thread->is_run == 0 ) {
        thread->is_run = 1;
    }
}

static void __join(Thread * thread,Thread * th)
{
    if ( th->joinable ) {
        th->joinable = 0;
        pthread_join(th->get_tid(th),NULL);
    }
}

static void __detach(Thread *thread) 
{
    if (thread->joinable) {
        pthread_detach(thread->tid);
    }
}

static int __get_tid(Thread *thread)
{
    return thread->tid;
}

static class_info_entry_t thread_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Thread, construct, __construct),
    Init_Nfunc_Entry(2 , Thread, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Thread, start, __start),
    Init_Vfunc_Entry(4 , Thread, set_start_routine, __set_start_routine),
    Init_Vfunc_Entry(5 , Thread, set_start_arg, __set_start_arg),
    Init_Vfunc_Entry(6 , Thread, set_opaque, __set_opaque),
    Init_Vfunc_Entry(7 , Thread, start_routine, __start_routine),
    Init_Vfunc_Entry(8 , Thread, stop, __stop),
    Init_Vfunc_Entry(9 , Thread, run, __run),
    Init_Vfunc_Entry(10, Thread, set_run_routine, __set_run_routine),
    Init_Vfunc_Entry(11, Thread, get_status, __get_status),
    Init_Vfunc_Entry(12, Thread, join, __join),
    Init_Vfunc_Entry(13, Thread, detach, __detach),
    Init_Vfunc_Entry(14, Thread, get_tid, __get_tid),
    Init_End___Entry(15, Thread),
};
REGISTER_CLASS("Thread", thread_class_info);

void *test_func(void *arg)
{
    dbg_str(DBG_DETAIL, "thread callback, arg addr:%p", arg);
    
}

int test_obj_thread()
{
    Thread *thread;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(DBG_DETAIL, "configurator_t addr:%p", c);
    /*
     *cfg_config(c, "/Thread", CJSON_STRING, "name", "alan thread") ;  
     */

    thread = OBJECT_NEW(allocator, Thread, NULL);

    dbg_str(DBG_DETAIL, "thread addr:%p", thread);
    dbg_str(DBG_DETAIL, "allocator addr:%p", allocator);

    object_dump(thread, "Thread", buf, 2048);
    dbg_str(OBJ_DETAIL, "Thread dump: %s", buf);
    thread->set_start_routine(thread, test_func);
    thread->set_start_arg(thread, thread);
    thread->set_opaque(thread, allocator);
    thread->start(thread);

    pause();

    object_destroy(thread);
    cfg_destroy(c);
}

static void *func(void *arg)
{
    Thread * thread = (Thread *)arg;
    int i = 0;
    while (i < 200 ) {
        usleep(10000);
        i++;
        if ( i == 150 ) {
            thread->stop(thread);
        }
        dbg_str(DBG_IMPORTANT," i= %d   I= %d " ,i,thread->tid);
    }
    
    return 1;
}


static void *func_detach(void *arg)
{
    Thread * thread = (Thread *)arg;
    thread->detach(thread);
    int i = 0;
    while (i < 200 ) {
        usleep(10000);
        i++;
        if ( i == 150 ) {
            thread->stop(thread);
        }
        dbg_str(DBG_IMPORTANT," i= %d   I= %d " ,i,thread->tid);
    }
    
    return 1;
}

static void * join_func(void *arg)
{
    dbg_str(DBG_IMPORTANT,"JOIN FUNC");
    return NULL;
}


static void *func_detach2(void *arg)
{
    Thread * thread = (Thread *)arg;
    //hread->detach(thread);
    int i = 0;
    while (i < 200 ) {
        usleep(10000);
        i++;
        if ( i == 150 ) {
            thread->stop(thread);
        }
        dbg_str(DBG_IMPORTANT," i= %d   I= %d " ,i,thread->tid);
    }
    
    return 1;
}


static int test_safe_thread()
{
    Thread *thread;
    Thread *thread_join;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;      
    thread = OBJECT_NEW(allocator, Thread, NULL);
    thread_join = OBJECT_NEW(allocator, Thread, NULL);
    thread->set_run_routine(thread,func);
    thread->start(thread);
    thread_join->set_run_routine(thread_join,join_func);
    thread_join->start(thread);
    thread_join->join(thread_join,thread);
   // pthread_join(thread->get_tid(thread),NULL);
    return 1;
}

static int test_thread_join()
{
    Thread *thread;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;      
    thread = OBJECT_NEW(allocator, Thread, NULL);
    thread->set_run_routine(thread,func);
    thread->start(thread);

    //thread->join(thread);
    dbg_str(DBG_ERROR," main thread wait sub thread!!!!!!!!!!");
    return 1;
}

static int  test_thread_detach1()
{
    Thread *thread;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;      
    thread = OBJECT_NEW(allocator, Thread, NULL);
    thread->set_run_routine(thread,func_detach);
    thread->start(thread);


    sleep(5);
    return 1;
}

static int  test_thread_detach2()
{
    Thread *thread;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;      
    thread = OBJECT_NEW(allocator, Thread, NULL);
    thread->set_run_routine(thread,func_detach2);
    thread->start(thread);
    //thread->detach(thread);  
 
    sleep(5);
    return 1;
}



REGISTER_STANDALONE_TEST_FUNC(test_obj_thread);
REGISTER_STANDALONE_TEST_FUNC(test_safe_thread);
REGISTER_STANDALONE_TEST_FUNC(test_thread_join);
REGISTER_STANDALONE_TEST_FUNC(test_thread_detach1);
REGISTER_STANDALONE_TEST_FUNC(test_thread_detach2);
