/**
 * @file io_user.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 1.0
 * @date 2016-09-06
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>     /* basic system data types */
#include <sys/socket.h>    /* basic socket definitions */
#include <netinet/in.h>    /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>     /* inet(3) functions */
#include <sys/epoll.h>     /* epoll function */
#include <fcntl.h>         /* nonblocking */
#include <sys/resource.h>  /*setrlimit */
#include <signal.h>
#include <libobject/utils/concurrent/concurrent.h>
#include <libobject/utils/concurrent/io_user.h>


io_user_t *io_user(allocator_t *allocator,
             int user_fd,
             uint8_t user_type,
             void (*io_event_handler)(int fd, short event, void *arg),
             void (*slave_work_function)(concurrent_slave_t *slave,void *arg),
             int (*process_task_cb)(void *task),
             void *opaque)
{
    concurrent_t *c = concurrent_get_global_concurrent_addr();
    io_user_t *io_user = NULL;

    if ((io_user = (io_user_t *)allocator_mem_alloc(
                    allocator, sizeof(io_user_t))) == NULL)
    {
        dbg_str(DBG_ERROR,"io_user_create err");
        return NULL;
    }

    io_user->allocator           = allocator;
    io_user->user_fd             = user_fd;
    io_user->user_type           = user_type;
    io_user->opaque              = opaque;
    io_user->slave_work_function = slave_work_function;
    io_user->io_event_handler    = io_event_handler;
    io_user->master              = c->master;
    io_user->process_task_cb     = process_task_cb;
    dbg_str(DBG_DETAIL,"io_user->master=%p,allocator=%p",io_user->master,allocator);

    concurrent_add_event_to_master(c,
                                   io_user->user_fd,//int fd,
                                   EV_READ | EV_PERSIST,//int event_flag,
                                   /*
                                    *EV_READ,//int event_flag,
                                    */
                                   &io_user->event,//struct event *event, 
                                   NULL,
                                   io_user->io_event_handler,//void (*event_handler)(int fd, short event, void *arg),
                                   io_user);//void *arg);

    return io_user;
}
int io_user_destroy(io_user_t *io_user)
{
    concurrent_t *c = concurrent_get_global_concurrent_addr();

    concurrent_del_event_of_master(c,
                                   &io_user->event);

    allocator_mem_free(io_user->allocator,io_user);

    return 0;
}
