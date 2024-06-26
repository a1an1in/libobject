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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Thread.h>

static int __construct(Thread *thread, char *init_str)
{
    allocator_t *allocator = thread->obj.allocator;

    dbg_str(DBG_INFO, "Thread construct, thread addr:%p", thread);
    thread->is_run   = 0;
    thread->arg      = NULL;
    thread->joinable = 1;

    return 0;
}

static int __deconstrcut(Thread *thread)
{
    int ret;
    void *tret;

    dbg_str(DBG_INFO, "thread deconstruct, thread addr:%p", thread);

    if (thread->joinable) {
        ret = pthread_join(thread->tid, &tret);
        if (ret != 0) {
            dbg_str(OBJ_WARN, "can't join with thread tid=%d", thread->tid);
        }
    }

    dbg_str(DBG_INFO, "thread deconstruct, out");

    return 0;
}

static int __start(Thread *thread)
{
    void *arg;
    int ret = 0;

    TRY {
        THROW_IF(thread->is_run, -1);
        THROW_IF(thread->start_routine == NULL, -1);

        if (thread->arg == NULL) {
            thread->arg = thread;
        }
        if ((pthread_create(&thread->tid, NULL, thread->start_routine, thread->arg)) != 0) {
            dbg_str(OBJ_WARN, "pthread start error");
        }

        thread->is_run = 0;
    } CATCH (ret) {}

    return ret;
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
    dbg_str(OBJ_WARN, "start_routine, you may need implent it!!");
    return NULL;
}

static void __stop(Thread * thread)
{
    if (thread->is_run) {
        thread->is_run = 0;
    }
}

static int __get_status(Thread *thread)
{
    return thread->is_run;
}

static void __run(Thread *thread)
{
    if (thread->is_run == 0) {
        thread->is_run = 1;
    }
}

static void __join(Thread * thread,Thread * th)
{
    if (th->joinable) {
        th->joinable = 0;
        pthread_join(th->get_tid(th),NULL);
    }
}

static void __detach(Thread *thread) 
{
    if (thread->joinable) {
        pthread_detach(thread->tid);
        thread->joinable = 0;
    }
}

static pthread_t __get_tid(Thread *thread)
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
    Init_Vfunc_Entry(10, Thread, get_status, __get_status),
    Init_Vfunc_Entry(11, Thread, join, __join),
    Init_Vfunc_Entry(12, Thread, detach, __detach),
    Init_Vfunc_Entry(13, Thread, get_tid, __get_tid),
    Init_End___Entry(14, Thread),
};
REGISTER_CLASS(Thread, thread_class_info);

#if 0

void *test_func(void *arg)
{
    dbg_str(DBG_DETAIL, "thread callback, arg addr:%p", arg);
    
}

int test_obj_thread()
{
    Thread *thread;
    allocator_t *allocator = allocator_get_default_instance();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    thread = OBJECT_NEW(allocator, Thread, NULL);

    dbg_str(DBG_DETAIL, "thread addr:%p", thread);
    dbg_str(DBG_DETAIL, "allocator addr:%p", allocator);

    object_dump(thread, "Thread", buf, 2048);
    dbg_str(OBJ_DETAIL, "Thread dump: %s", buf);
    thread->set_start_routine(thread, test_func);
    thread->set_start_arg(thread, thread);
    thread->set_opaque(thread, allocator);
    thread->start(thread);

#if (defined(WINDOWS_USER_MODE))
    system("pause"); 
#else
    pause();
#endif

    object_destroy(thread);
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
        dbg_str(DBG_INFO," i= %d   I= %d " ,i,thread->tid);
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
        dbg_str(DBG_INFO," i= %d   I= %d " ,i,thread->tid);
    }
    
    return 1;
}

static void * join_func(void *arg)
{
    dbg_str(DBG_INFO,"JOIN FUNC");
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
        dbg_str(DBG_INFO," i= %d   I= %d " ,i,thread->tid);
    }
    
    return 1;
}

static int test_safe_thread()
{
    Thread *thread;
    Thread *thread_join;

    allocator_t *allocator = allocator_get_default_instance();
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
    allocator_t *allocator = allocator_get_default_instance();

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
    allocator_t *allocator = allocator_get_default_instance();
    thread = OBJECT_NEW(allocator, Thread, NULL);
    thread->set_run_routine(thread,func_detach);
    thread->start(thread);

    sleep(5);

    return 1;
}

static int  test_thread_detach2()
{
    Thread *thread;
    allocator_t *allocator = allocator_get_default_instance();
    thread = OBJECT_NEW(allocator, Thread, NULL);
    thread->set_run_routine(thread,func_detach2);
    thread->start(thread);
    sleep(5);

    return 1;
}
REGISTER_TEST_CMD(test_obj_thread);
REGISTER_TEST_CMD(test_safe_thread);
REGISTER_TEST_CMD(test_thread_join);
REGISTER_TEST_CMD(test_thread_detach1);
REGISTER_TEST_CMD(test_thread_detach2);
#endif
