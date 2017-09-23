/**
 * @file timer.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 1.0
 * @date 2016-09-19
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <libobject/utils/dbg/debug.h>
#include <libobject/utils/concurrent/concurrent.h>
#include <libobject/utils/concurrent/io_user.h>
#include <libobject/utils/concurrent/tmr_user.h>
#include <libobject/utils/timeval/timeval.h>

int event_is_persistent;

static void slave_work_function(concurrent_slave_t *slave,void *arg)
{
    tmr_user_t *timer = (tmr_user_t *)arg;

    dbg_str(NET_DETAIL,"slave_work_function begin");
    timer->process_timer_task_cb(timer);
    dbg_str(NET_DETAIL,"slave_work_function end");

    return ;
}

void timer_event_handler(int fd, short event, void *arg)
{

    tmr_user_t *timer = (tmr_user_t *)arg;
    concurrent_master_t *master = timer->master;
    struct concurrent_message_s message;

    dbg_str(NET_DETAIL,"timer event handler begin");
    master->assignment_count++;//do for assigning slave
    concurrent_master_init_message(&message,
                                   timer->slave_work_function,
                                   timer,
                                   0);
    concurrent_master_add_message(master,&message);
    dbg_str(NET_DETAIL,"timer event handler end");

    return ;
}

tmr_user_t *tmr_user(allocator_t *allocator,
                     struct timeval *tv,
                     uint16_t timer_flags,
                     void (*process_timer_task_cb)(void *task),
                     void *opaque)
{
    concurrent_t *c = concurrent_get_global_concurrent_addr();
    tmr_user_t *tmr_user = NULL;

    if ((tmr_user = (tmr_user_t *)allocator_mem_alloc(
                    allocator, sizeof(tmr_user_t))) == NULL)
    {
        dbg_str(DBG_ERROR,"tmr_user_create err");
        return NULL;
    }

    tmr_user->allocator             = allocator;
    tmr_user->tmr_user_fd           = -1;
    tmr_user->opaque                = opaque;
    tmr_user->tmr_event_handler     = timer_event_handler;
    tmr_user->slave_work_function   = slave_work_function;
    tmr_user->process_timer_task_cb = process_timer_task_cb;
    tmr_user->master                = c->master;
    tmr_user->flags                 = timer_flags;
    tmr_user->tv                    = *tv;

    concurrent_add_event_to_master(c,
                                   -1,//int fd,
                                   tmr_user->flags,//int event_flag,
                                   &tmr_user->event,//struct event *event, 
                                   &tmr_user->tv,
                                   tmr_user->tmr_event_handler,
                                   tmr_user);//void *arg);

    return tmr_user;
}

tmr_user_t *tmr_user_reuse(tmr_user_t * tmr_user)
{
    concurrent_t *c = concurrent_get_global_concurrent_addr();

    dbg_str(DBG_DETAIL,"state_machine_reuse_timer");
    concurrent_add_event_to_master(c,
                                   -1,//int fd,
                                   tmr_user->flags,//int event_flag,
                                   &tmr_user->event,//struct event *event, 
                                   &tmr_user->tv,
                                   tmr_user->tmr_event_handler,
                                   tmr_user);//void *arg);

    return tmr_user;
}

int tmr_user_destroy(tmr_user_t *tmr_user)
{
    concurrent_t *c = concurrent_get_global_concurrent_addr();

    dbg_str(DBG_DETAIL,"state_machine_destroy_timer");
    concurrent_del_event_of_master(c,
                                   &tmr_user->event);
    allocator_mem_free(tmr_user->allocator,tmr_user);

    return 0;
}

int tmr_user_stop(tmr_user_t *tmr_user)
{
    concurrent_t *c = concurrent_get_global_concurrent_addr();

    dbg_str(DBG_DETAIL,"state_machine_stop_timer");
    concurrent_del_event_of_master(c,
                                   &tmr_user->event);

    return 0;
}

static void test_process_timer_task_callback(void *timer)
{
    dbg_str(DBG_DETAIL,"process_timer_task_callback begin");
    dbg_str(DBG_DETAIL,"process_timer_task_callback end");
}

int test_tmr_user()
{
    allocator_t *allocator = allocator_get_default_alloc();
    tmr_user_t *timer;
    struct timeval tv;

    dbg_str(DBG_DETAIL,"test_tmr_user");

    timeval_clear(&tv);
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    timer = tmr_user(allocator,
                     &tv,
                     /*
                      *0,
                      */
                     EV_PERSIST,
                     test_process_timer_task_callback,
                     NULL);
    /*
     *tmr_user_stop(timer);
     */

    /*
     *int count = 0;
     *while(count ++ < 1000) {
     *    timer->tv.tv_sec = 1;
     *    tmr_user_reuse(timer);
     *    tmr_user_stop(timer);
     *}
     */

    /*
     *tmr_user_reuse(timer);
     */

    /*
     *pause();
     */
    while (1) sleep(5);
    
    tmr_user_stop(timer);
    tmr_user_destroy(timer);

    return 0;
}

