/**
 * @file concurrent.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-03-11
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
#include <libobject/utils/dbg/debug.h>
#include <libobject/utils/concurrent/concurrent.h>
#include <libobject/attrib_priority.h>

concurrent_t *global_concurrent;
uint8_t g_slave_amount = 1;
uint8_t g_concurrent_lock_type = 0;

concurrent_t *concurrent_get_global_concurrent_addr()
{
    return global_concurrent;
}

concurrent_ta_t *
concurrent_ta_create(allocator_t *allocator)
{
    concurrent_ta_t *task_admin = NULL;

    task_admin = (concurrent_ta_t *)allocator_mem_alloc(allocator,
                                                        sizeof(concurrent_ta_t));
    if (task_admin == NULL) {
        dbg_str(CONCURRENT_ERROR,"allocc concurrent task admin err");
        exit(1);
    }
    task_admin->allocator = allocator;

    return task_admin;
}

int 
concurrent_ta_init(concurrent_ta_t *task_admin,
                   uint8_t key_size,
                   uint32_t value_size,
                   uint32_t bucket_size,
                   uint8_t admin_lock_type,
                   uint8_t hmap_lock_type)
{
    int ret = 0;

    task_admin->admin_lock_type = admin_lock_type;
    task_admin->hmap_lock_type  = hmap_lock_type;
    task_admin->key_size        = key_size;
    task_admin->value_size      = value_size;
    task_admin->bucket_size     = bucket_size;

    /*
     *task_admin->hmap = hash_map_create(allocator,PTHREAD_MUTEX_LOCK);
     */
    task_admin->hmap = hash_map_alloc(task_admin->allocator);

    hash_map_init(task_admin->hmap,
                  key_size,//uint32_t key_size,
                  value_size,
                  bucket_size);
    //if concurrent work mode is process,then the lock need be modified!!!!
    sync_lock_init(&task_admin->admin_lock,admin_lock_type);

    return ret;
}

int concurrent_ta_add(concurrent_ta_t *task_admin,void *key,void *data)
{
    pair_t *pair;

    //malloc problem ,need modify
    
    pair = create_pair(task_admin->key_size,task_admin->value_size);
    make_pair(pair,key,data);
    hash_map_insert_data(task_admin->hmap,pair->data);
    destroy_pair(pair);

    return 0;
}

int 
concurrent_ta_search(concurrent_ta_t *task_admin,
                     void *key,
                     hash_map_pos_t *pos)
{
    return hash_map_search(task_admin->hmap,key,pos);
}

int 
concurrent_ta_del(concurrent_ta_t *task_admin,
                  hash_map_pos_t *pos)
{
    return hash_map_delete(task_admin->hmap,pos);
}

int 
concurrent_ta_del_by_key(concurrent_ta_t *task_admin,
                         void *key)
{
    hash_map_pos_t pos;
    int ret = 0;

    hash_map_search(task_admin->hmap, key,&pos);
    ret = hash_map_delete(task_admin->hmap, &pos);
    return ret;
}

int 
concurrent_ta_destroy(concurrent_ta_t *task_admin)
{
    hash_map_destroy(task_admin->hmap);
    sync_lock_destroy(&task_admin->admin_lock);
    allocator_mem_free(task_admin->allocator,task_admin);

    return 0;
}

/**
 * @synopsis slave_event_handler_process_message 
 *              tell slave do something,if listen fd,it may not need use this func,
 *              main thread can set slave event directly.
 * @param fd
 * @param event
 * @param arg
 */
static void 
slave_event_handler_process_message(int fd, short event, void *arg)
{
    concurrent_slave_t *slave = (concurrent_slave_t *)arg;
    char buf[1];          
    list_t *l;
    struct concurrent_message_s *message;

    /*
     *dbg_str(CONCURRENT_DETAIL,"slave_event_handler_process_message,fd=%d",fd);
     */
    if (read(fd, buf, 1) != 1) {
        dbg_str(CONCURRENT_WARNNING,"cannot read form pipe");
        return;
    }
    switch (buf[0]) {
        case 'c': 
            l = llist_detach_front(slave->message_que);
            message = (struct concurrent_message_s *)l->data;
            message->work_func(slave,message->task);
            allocator_mem_free(slave->allocator,l);
            break;
        case 'p':
            break;            
    }

}

void *
concurrent_slave_thread(void *arg)
{
    concurrent_slave_t *slave = (concurrent_slave_t *)arg;
    concurrent_master_t *master = slave->master;

    dbg_str(CONCURRENT_DETAIL,
            "concurrent_slave_thread start,concurrent_slave_thread id=%d",
            slave->work_id);

    slave->event_base = event_base_new();
    if (slave->event_base == NULL) {
        dbg_str(CONCURRENT_ERROR,"cannot create slave event_base");
        exit(1);
    }

    event_assign(&slave->message_event,
                 slave->event_base,
                 slave->rcv_notify_fd,
                 EV_READ | EV_PERSIST, 
                 slave_event_handler_process_message, 
                 arg);

    if (event_add(&slave->message_event, 0) == -1) {
        dbg_str(CONCURRENT_WARNNING,"event_add err");
    }

    master->concurrent_slave_inited_flag = 1;

    event_base_dispatch(slave->event_base);

    return NULL;
}

int 
concurrent_slave_add_new_event(concurrent_slave_t *slave,
                               int fd,int event_flag,
                               struct event *event,
                               void (*event_handler)(int fd, short event, void *arg),
                               void *task)
{
    event_assign(event,slave->event_base,fd, event_flag, event_handler, task);
    if (event_add(event, 0) == -1) {
        dbg_str(CONCURRENT_WARNNING,"event_add err");
    }

    return 0;
}

int 
__concurrent_master_create_slave(concurrent_master_t *master,uint8_t slave_id)
{
    int ret = 0;
    concurrent_slave_t *slave = &master->slave[slave_id];
    int fds[2];

    slave->message_que = master->message_que;
    slave->allocator   = master->allocator;
    slave->task_admin  = master->task_admin;
    slave->master      = master;

    if (master->concurrent_work_type == SERVER_WORK_TYPE_THREAD) {

        slave->work_id = slave_id;
        if (pipe(fds)) {
            dbg_str(CONCURRENT_ERROR,"cannot create pipe");
            exit(1);
        }
        slave->rcv_notify_fd = fds[0];
        master->snd_notify_fd[slave_id] = fds[1];

        if ((ret = pthread_create(&slave->id.tid, NULL, concurrent_slave_thread, slave)) != 0) {
            dbg_str(CONCURRENT_ERROR,"cannot slave pthread");
            exit(1);
        }

    } else if ( master->concurrent_work_type == SERVER_WORK_TYPE_PROCESS) {
    
    } else {
        dbg_str(CONCURRENT_ERROR,"not support concurrent_work_type");
        return -1;
    } 

    return ret;
}

int concurrent_master_create_slaves(concurrent_master_t *master)
{
    int i = 0, ret = 0;

    master->slave = (concurrent_slave_t *)allocator_mem_alloc(master->allocator,
                    sizeof(concurrent_slave_t) * master->slave_amount);
    if (master->slave == NULL) {
        dbg_str(CONCURRENT_ERROR,"alloc slave err");
        ret = -1;
        goto end;
    }

    master->snd_notify_fd = (int *)allocator_mem_alloc(master->allocator,
                            sizeof(int) * master->slave_amount);
    if (master->slave == NULL) {
        dbg_str(CONCURRENT_ERROR,"alloc snd_notify_fd err");
        ret = -1;
        goto end;
    }

    for (i = 0; i < master->slave_amount; i++) {
        ret = __concurrent_master_create_slave(master, i);
    }

    while(master->concurrent_slave_inited_flag != 1);

end:
    return ret;

}

int concurrent_master_destroy_slaves(concurrent_master_t *master)
{
    allocator_mem_free(master->allocator, master->slave);
    allocator_mem_free(master->allocator, master->snd_notify_fd);
}

concurrent_master_t *
concurrent_master_create(allocator_t *allocator)
{
    concurrent_master_t *master;

    master = (struct concurrent_master_s *)allocator_mem_alloc(allocator,
                                                               sizeof(struct concurrent_master_s));
    master->allocator                     = allocator;
    master->concurrent_master_inited_flag = 0;
    master->concurrent_slave_inited_flag  = 0;
    master->assignment_count              = 0;

    return master;
}
/**
 * @synopsis master_event_handler_add_new_event 
 *
 * @param fd 
 * @param event
 * @param arg
 */
static void 
master_event_handler_add_new_event(int fd,short event,void *arg)
{
    concurrent_master_t *master  = (concurrent_master_t *)arg;
    list_t *l;
    struct concurrent_message_s *message;
    char buf[1];          

    if (read(fd, buf, 1) != 1) {
        dbg_str(CONCURRENT_WARNNING,"cannot read form pipe");
        return;
    }
    switch (buf[0]) {
        case 'r': 
            dbg_str(CONCURRENT_DETAIL,"master_event_handler_add event");
            dbg_str(DBG_VIP,"master addr:%p, llist new_ev_que:%p, lock:%p, lock ops:%p", 
                    master,
                    master->new_ev_que, &master->new_ev_que->list_lock, 
                    master->new_ev_que->list_lock.lock_ops);

            l = llist_detach_front(master->new_ev_que);
            dbg_str(DBG_DETAIL,"run at here");
            message = (struct concurrent_message_s *)l->data;

            event_assign(message->event,
                         message->ev_base,
                         message->ev_fd, 
                         message->ev_events, 
                         message->ev_callback, 
                         message->ev_callback_arg);

            if (event_add(message->event, message->tv) == -1) {
                dbg_str(CONCURRENT_WARNNING,"event_add err");
            }

            allocator_mem_free(master->allocator,l);
            break;
        case 'd': 
            l = llist_detach_front(master->new_ev_que);
            message = (struct concurrent_message_s *)l->data;

            dbg_str(CONCURRENT_DETAIL,
                    "master_event_handler_del event,ev_flags=%x,event addr:%p",
                    ((struct event *)(message->event))->ev_flags,
                    message->event);

            if (event_del(message->event) < 0) {
                dbg_str(CONCURRENT_WARNNING,"event_del err");
            }

            allocator_mem_free(master->allocator,l);
            break;
        case 'p':
            break;            
    }

}

void *concurrent_master_thread(void *arg)
{
    concurrent_master_t *master  = (concurrent_master_t *)arg;
    struct event event;

    dbg_str(CONCURRENT_DETAIL,"concurrent_master_thread start");

    master->event_base = event_base_new();
    /*
     *listenning rcv_add_new_event_fd, we can designe any right fd here,
     *we just need event base become loop state.
     */
    event_assign(&event,
                 master->event_base,
                 master->rcv_add_new_event_fd,
                 EV_READ | EV_PERSIST,
                 master_event_handler_add_new_event,
                 master);

    if (event_add(&event, 0) == -1) {
        dbg_str(CONCURRENT_WARNNING,"event_add err");
    }

    dbg_str(CONCURRENT_DETAIL,"concurrent_master_thread end");
    master->concurrent_master_inited_flag = 1;
    event_base_dispatch(master->event_base);

    return NULL;
}

int 
concurrent_master_init(concurrent_master_t *master,
                       uint8_t concurrent_work_type,
                       uint32_t task_size,
                       uint8_t slave_amount)
{
    int ret = 0;
    int fds[2];
    int data_size = sizeof(struct concurrent_message_s);
    int lock_type = 1;

    dbg_str(CONCURRENT_DETAIL,"concurrent_master_init");
    master->concurrent_work_type = concurrent_work_type;
    master->slave_amount         = slave_amount;
    master->message_count        = 0;
    master->message_que          = llist_alloc(master->allocator);
    llist_set(master->message_que,"lock_type",&lock_type);
    llist_set(master->message_que,"data_size",&data_size);
    llist_init(master->message_que);

    // create task admin
    master->task_admin = concurrent_ta_create(master->allocator);
    ret = concurrent_ta_init(master->task_admin,
                             4,
                             task_size,
                             10,//uint32_t bucket_size,
                             1,//uint8_t admin_lock_type,
                             0);//uint8_t hmap_lock_type)

    if (pipe(fds)) {
        dbg_str(CONCURRENT_ERROR,"cannot create pipe");
        exit(1);
    }

    master->rcv_add_new_event_fd = fds[0];
    master->snd_add_new_event_fd = fds[1];

    if ((ret = pthread_create(&master->id.tid,
                              NULL, 
                              concurrent_master_thread, 
                              master)) != 0) 
    {
        dbg_str(CONCURRENT_ERROR,"cannot slave pthread");
        exit(1);
    }

    concurrent_master_create_slaves(master);
    return ret;
}

int 
concurrent_master_add_task_and_message(concurrent_master_t *master,
                                       void *task,
                                       void *key,
                                       void (*work_func)(concurrent_slave_t *slave,void *arg))
{
    struct concurrent_message_s message;
    hash_map_pos_t pos;
    void *t;

    t = concurrent_master_add_task(master,task,key);
    concurrent_master_init_message(&message, work_func,t,0);
    concurrent_master_add_message(master,&message);

    return 0;
}

void *
concurrent_master_add_task(concurrent_master_t *master,
                           void *task,
                           void *key)
{
    hash_map_pos_t pos;
    void *t = NULL;

    sync_lock(&master->task_admin->admin_lock,NULL);
    concurrent_ta_add(master->task_admin,key,task);
    master->assignment_count++;

    concurrent_ta_search(master->task_admin,key,&pos);
    if (pos.hlist_node_p == NULL) {
        dbg_str(CONCURRENT_ERROR,"not found key,key=%s",key);
        return NULL;
    }

    t = hash_map_pos_get_pointer(&pos);
    sync_unlock(&master->task_admin->admin_lock);

    return t;
}

int concurrent_master_choose_slave(concurrent_master_t *master)
{
    return master->assignment_count % master->slave_amount;
}

static void 
concurrent_master_notify_slave(concurrent_master_t *master,char command)
{
    int i = 0;

    i = concurrent_master_choose_slave(master);
    dbg_str(CONCURRENT_DETAIL,
            "concurrent_master_notify_slave,slave i=%d is assigned,notify fd=%d",
            i,master->snd_notify_fd[i]);

    if (write(master->snd_notify_fd[i], &command, 1) != 1) {
        dbg_str(CONCURRENT_WARNNING,"concurrent_master_notify_slave,write pipe err");
    }
}

int concurrent_master_add_message(concurrent_master_t *master,
                                  struct concurrent_message_s *message)
{
    char command = 'c';

    llist_push_back(master->message_que,message);
    concurrent_master_notify_slave(master,command);

    return 0;
}

int 
concurrent_master_init_message(struct concurrent_message_s *message,
                               void (*work_func)(concurrent_slave_t *slave,void *arg),
                               void *task,
                               int32_t message_id)
{
    message->work_func  = work_func;
    message->task       = task;
    message->message_id = message_id;

    return 0;
}

//there may be some omited,must check carefully later
int concurrent_master_destroy(concurrent_master_t *master)
{
    concurrent_ta_destroy(master->task_admin);
    concurrent_master_destroy_slaves(master);
    llist_destroy(master->message_que);
    allocator_mem_free(master->allocator,master);
    return 0;
}

concurrent_t *
concurrent_create(allocator_t *allocator)
{
    concurrent_t *c;

    if ((c = (concurrent_t *)allocator_mem_alloc(allocator,
                                                 sizeof(concurrent_t))) == NULL)
    {
        dbg_str(CONCURRENT_ERROR,"concurrent_create err");
        return NULL;
    }
    c->allocator= allocator;
    dbg_str(CONCURRENT_DETAIL,"concurrent allocator=%p",allocator);

    return c;
}

int 
concurrent_init(concurrent_t *c,
                uint8_t concurrent_work_type,
                uint32_t task_size,
                uint8_t slave_amount,
                uint8_t concurrent_lock_type)
{
    int ret = 0;
    int data_size = sizeof(struct concurrent_message_s);
    int lock_type = 1;

    dbg_str(CONCURRENT_DETAIL,"concurrent_init");
    c->master = concurrent_master_create(c->allocator);
    dbg_str(CONCURRENT_DETAIL,"concurrent master=%p",c->master);

    concurrent_master_init(c->master, 
                           concurrent_work_type, 
                           task_size, 
                           slave_amount);

    c->snd_add_new_event_fd = c->master->snd_add_new_event_fd;

    c->new_ev_que           = llist_alloc(c->allocator);
    c->master->new_ev_que   = c->new_ev_que;

    llist_set(c->new_ev_que,"lock_type",&lock_type);
    llist_set(c->new_ev_que,"data_size",&data_size);
    llist_init(c->new_ev_que);

    dbg_str(DBG_VIP,"master addr:%p, llist new_ev_que:%p, lock:%p, lock ops:%p", 
            c->master,
            c->new_ev_que, &c->new_ev_que->list_lock, 
            c->new_ev_que->list_lock.lock_ops);


    dbg_str(CONCURRENT_DETAIL,"concurrent_init, lock func:%p", c->new_ev_que->list_lock.lock_ops->sync_lock);

    sync_lock_init(&c->concurrent_lock,concurrent_lock_type);

    return ret;
}

int 
concurrent_add_event_to_master(concurrent_t *c,
                               int fd,
                               int event_flag,
                               struct event *event,
                               struct timeval *tv,
                               void (*event_handler)(int fd, short event, void *arg),
                               void *arg)
{
    struct concurrent_message_s message;

    while(c->master->concurrent_master_inited_flag != 1);

    dbg_str(CONCURRENT_IMPORTANT,"concurrent_add_new_event to master");

    message.event           = event;
    message.ev_base         = c->master->event_base;
    message.ev_fd           = fd;
    message.ev_events       = event_flag;
    message.ev_callback     = event_handler;
    message.ev_callback_arg = arg;
    message.tv              = tv;

    llist_push_back(c->new_ev_que,&message);

    if (write(c->snd_add_new_event_fd, "r", 1) != 1) {
        dbg_str(CONCURRENT_ERROR,"cannot write pipe");
    }

    return 0;
}

int 
concurrent_del_event_of_master(concurrent_t *c,
                               struct event *event)
{
    struct concurrent_message_s message;

    while(c->master->concurrent_master_inited_flag != 1);

    message.event = event;

    dbg_str(CONCURRENT_DETAIL,
            "concurrent_del_event_of_master,ev_flags=%x,event addr:%p",
            ((struct event *)(message.event))->ev_flags,
            message.event);

    llist_push_back(c->new_ev_que,&message);

    if (write(c->snd_add_new_event_fd, "d", 1) != 1) {
        dbg_str(CONCURRENT_ERROR,"cannot write pipe");
    }

    return 0;
}

void concurrent_destroy(concurrent_t *c)
{
    concurrent_master_destroy(c->master);
    llist_destroy(c->new_ev_que);

    allocator_mem_free(c->allocator,c);

    /*
     *allocator_mem_info(c->allocator);
     */
}

#define MAX_PROXY_TASK_SIZE 128
__attribute__((constructor(ATTRIB_PRIORITY_CONCURRENT))) void
concurrent_constructor()
{
    uint8_t slave_amount = g_slave_amount;
    uint8_t lock_type    = g_concurrent_lock_type;
    concurrent_t *c;

    ATTRIB_PRINT("constructor ATTRIB_PRIORITY_CONCURRENT=%d,construct concurrent\n",
                 ATTRIB_PRIORITY_CONCURRENT);

#if 0
    allocator_t *allocator;
    /*
     *if ((allocator = allocator_create(ALLOCATOR_TYPE_SYS_MALLOC,0) ) == NULL) {
     *    dbg_str(CONCURRENT_ERROR,"proxy_create allocator_create err");
     *    return;
     *}
     */
    allocator = allocator_create(ALLOCATOR_TYPE_CTR_MALLOC,0);
    allocator_ctr_init(allocator, 0, 32, 1024);
#else
    allocator_t *allocator = allocator_get_default_alloc();
#endif


    c = concurrent_create(allocator);

    concurrent_init(c,
                    SERVER_WORK_TYPE_THREAD,
                    MAX_PROXY_TASK_SIZE, 
                    slave_amount, 
                    lock_type);//uint8_t concurrent_lock_type);

    global_concurrent = c;
}

__attribute__((destructor(ATTRIB_PRIORITY_CONCURRENT))) void 
concurrent_destructor()
{
    concurrent_t *c = concurrent_get_global_concurrent_addr();

    ATTRIB_PRINT("destructor ATTRIB_PRIORITY_CONCURRENT=%d,concurrent destructor\n",
                 ATTRIB_PRIORITY_CONCURRENT);

    concurrent_destroy(c);
}
