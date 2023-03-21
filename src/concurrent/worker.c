/**
 * @file io_worker.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-10-23
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
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/Worker.h>
#include <libobject/concurrent/Producer.h>

Worker *
io_worker(allocator_t *allocator, int fd, 
          struct timeval *ev_tv, void *ev_callback, 
          void *work_callback, Producer *producer, 
          void *opaque)
{
    Worker *worker = NULL;

    if (producer == NULL) {
        producer = concurrent_get_default_instance();
    }
    worker = OBJECT_NEW(allocator, Worker, NULL);
    worker->opaque = opaque;

    worker->assign(worker, fd, EV_READ | EV_PERSIST, ev_tv, 
                   ev_callback, worker, work_callback);
    worker->enroll(worker, producer);

    return worker;
}

static void
signal_worker_cb(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;

    worker->work_callback(worker->opaque);

    return;
}

Worker *signal_worker(allocator_t *allocator, 
                      int fd, void *work_callback, void *opaque)
{
    Producer *producer = concurrent_get_default_instance();
    Worker *worker = NULL;

    worker = OBJECT_NEW(allocator, Worker, NULL);
    worker->opaque = opaque;

    worker->assign(worker, fd, EV_SIGNAL|EV_PERSIST, NULL, 
                   signal_worker_cb, worker, 
                   work_callback);
    worker->enroll(worker, producer);

    return worker;
}

struct timeval lasttime;
static void
timer_worker_timeout_cb(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    struct timeval newtime, difference;
    double elapsed;

    gettimeofday(&newtime, NULL);
    timeval_sub(&newtime, &lasttime, &difference);

    elapsed  = difference.tv_sec + (difference.tv_usec / 1.0e6);
    lasttime = newtime;

    dbg_str(DBG_SUC, "timeout_cb called at %d: %.3f seconds elapsed.", 
            (int)newtime.tv_sec, elapsed);
    dbg_str(DBG_DETAIL, "arg addr:%p", arg);
    worker->work_callback(worker->opaque);

    return;
}

Worker *timer_worker(allocator_t *allocator, int ev_events,
                     struct timeval *ev_tv, 
                     void *work_callback, void *opaque)
{
    Producer *producer = concurrent_get_default_instance();
    Worker *worker = NULL;

    worker = OBJECT_NEW(allocator, Worker, NULL);
    worker->opaque = opaque;

    worker->assign(worker, -1, ev_events, ev_tv, 
                   timer_worker_timeout_cb, worker, 
                   work_callback);
    worker->enroll(worker, producer);

    return worker;
}

int worker_destroy(Worker *worker)
{
    if (worker == NULL) return 0;
    worker->resign(worker);
    return object_destroy(worker);
}

