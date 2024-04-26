/**
 * @file concurrent.c
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
#include <libobject/concurrent/Worker.h>
#include <libobject/concurrent/Producer.h>

static int __construct(Worker *worker, char *init_str)
{
    dbg_str(EV_DETAIL, "worker construct, worker addr:%p", worker);
    worker->flags = 0;

    return 0;
}

static int __deconstrcut(Worker *worker)
{
    dbg_str(EV_DETAIL, "worker deconstruct, worker addr:%p", worker);
    worker->resign(worker);

    return 0;
}

static int  __assign(Worker *worker, int fd, int ev_events, 
                     struct timeval *ev_tv, void *ev_callback, 
                     void *ev_arg, void *work_callback)
{
    event_t *event = &worker->event;

    event->ev_fd          = fd;
    event->ev_events      = ev_events;
    event->ev_callback    = ev_callback;
    event->ev_arg         = ev_arg;
    worker->work_callback = work_callback;
    worker->flags         = 0;  //是否已经ressign标记。

    if (ev_tv != NULL)
        event->ev_timeout     = *ev_tv;
}

static int __enroll(Worker *worker, void *producer)
{
    Producer *p = (Producer *)producer;

    worker->producer = producer;
    p->add_worker(p, worker);

    return 0;
}

static int __resign(Worker *worker)
{
    Producer *p = worker->producer;
    
    if (p == NULL) {
        dbg_str(EV_WARN, "worker resign a unenrolled worker");
        return 0;
    }

    if (worker->flags & 1) {
        return 0; //has resigned
    }

    p->del_worker(p, worker);
    worker->flags |= 1;

    return 1;
}

static class_info_entry_t worker_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Worker, construct, __construct),
    Init_Nfunc_Entry(2 , Worker, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Worker, assign, __assign),
    Init_Vfunc_Entry(4 , Worker, enroll, __enroll),
    Init_Vfunc_Entry(5 , Worker, resign, __resign),
    Init_End___Entry(6 , Worker),
};
REGISTER_CLASS("Worker", worker_class_info);

static void
test_timeout_cb(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    struct timeval newtime, difference;
    double elapsed;
    static struct timeval lasttime;

    gettimeofday(&newtime, NULL);
    timeval_sub(&newtime, &lasttime, &difference);

    elapsed  = difference.tv_sec + (difference.tv_usec / 1.0e6);
    lasttime = newtime;

    dbg_str(DBG_SUC, "timeout_cb called at %d: %.3f seconds elapsed.", 
            (int)newtime.tv_sec, elapsed);
    dbg_str(DBG_DETAIL, "arg addr:%p", arg);
    worker->work_callback(NULL);
}

static void test_work_callback(void *task)
{
    dbg_str(DBG_SUC, "process timer task");
}
void test_obj_worker()
{
    Worker *worker;
    Producer *producer;
    allocator_t *allocator = allocator_get_default_instance();
    struct timeval ev_tv;

    ev_tv.tv_sec  = 2;
    ev_tv.tv_usec = 0;

    producer = OBJECT_NEW(allocator, Producer, NULL);
    producer->start(producer);
    sleep(1);

    worker = OBJECT_NEW(allocator, Worker, NULL);
    dbg_str(DBG_DETAIL, "worker addr:%p", worker);
    worker->assign(worker, -1, EV_READ | EV_PERSIST, 
                   &ev_tv, test_timeout_cb, worker, test_work_callback);
    worker->enroll(worker, producer);

#if (defined(WINDOWS_USER_MODE))
    system("pause"); 
#else
    pause();
#endif
    object_destroy(worker);
    object_destroy(producer);
}
