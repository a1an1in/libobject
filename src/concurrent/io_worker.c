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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/worker.h>
#include <libobject/concurrent/producer.h>

Worker *
io_worker(allocator_t *allocator, int fd, 
          struct timeval *ev_tv, void *ev_callback, 
          void *work_callback, Producer *producer, 
          void *opaque)
{
    Worker *worker = NULL;

    if (producer == NULL) {
        producer = global_get_default_producer();
    }
    worker = OBJECT_NEW(allocator, Worker, NULL);
    worker->opaque = opaque;

    worker->assign(worker, fd, EV_READ | EV_PERSIST, ev_tv, 
                   ev_callback, worker, work_callback);
    worker->enroll(worker, producer);

    return worker;
}

int io_worker_destroy(Worker *worker)
{   
    worker->resign(worker);
    return object_destroy(worker);
}

static void
test_pipe_ev_callback(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    char buf[255];
    int len;

    len = read(fd, buf, sizeof(buf) - 1);

    if (len == -1) {
        perror("read");
        return;
    } else if (len == 0) {
        fprintf(stderr, "Connection closed\n");
        return;
    }

    buf[len] = '\0';
    fprintf(stdout, "Read: %s\n", buf);

    worker->work_callback(NULL);

    return;
}

static void test_work_callback(void *task)
{
    dbg_str(DBG_SUC, "process io worker task");
}

static int create_fifo(char *name) 
{
    struct stat st;
    int socket;

    if (lstat(name, &st) == 0) {
        if ((st.st_mode & S_IFMT) == S_IFREG) {
            errno = EEXIST;
            perror("lstat");
            exit(1);
        }
    }

    unlink(name);
    if (mkfifo(name, 0600) == -1) {
        perror("mkname");
        exit(1);
    }

    /* Linux pipes are broken, we need O_RDWR instead of O_RDONLY */
    socket = open(name, O_RDWR | O_NONBLOCK, 0);

    if (socket == -1) {
        perror("open");
        exit(1);
    }
}

void test_obj_io_worker()
{
    allocator_t *allocator = allocator_get_default_alloc();
    Worker *worker;
    int fd;

    sleep(1);

    fd = create_fifo((char *)"event.fifo");
    worker = io_worker(allocator, fd, NULL, test_pipe_ev_callback, 
                       test_work_callback, NULL, NULL);
    pause();
    pause();
    io_worker_destroy(worker);
}

