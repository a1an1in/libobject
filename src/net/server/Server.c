/**
 * @file Server.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-10-24
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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/net/server/Server.h>
#include <libobject/core/Linked_List.h>

static void __release_working_worker(void *element)
{
    Worker *worker = (Worker *)element;

    worker->resign(worker);
    work_task_free(worker->task);
    object_destroy(worker->socket);
    object_destroy(worker);
}

static void __release_leisure_worker(void *element)
{
    Worker *worker = (Worker *)element;

    work_task_free(worker->task);
    object_destroy(worker->socket);
    object_destroy(worker);
}

static int __construct(Server *server, char *init_str)
{
    allocator_t *allocator = server->obj.allocator;

    server->working_workers = OBJECT_NEW(allocator, Linked_List, NULL);
    server->leisure_workers = OBJECT_NEW(allocator, Linked_List, NULL);

    return 0;
}

static int __deconstrcut(Server *server)
{
    List *working_list = server->working_workers;
    List *leisure_list = server->leisure_workers;

    working_list->for_each(working_list, __release_working_worker);
    object_destroy(working_list);

    leisure_list->for_each(leisure_list, __release_leisure_worker);
    object_destroy(leisure_list);

    return 0;
}

static int __bind(Server *server, char *host, char *service)
{
    Socket *socket = server->socket;

    return socket->bind(socket, host, service);
}

/*
 *every worker has its own task, so there has not concurrent problem.
 */
static ssize_t __new_conn_ev_callback(int fd, short event, void *arg)
{
    Worker *worker     = (Worker *)arg;
    Server *server     = (Server *)worker->opaque;
    List *working_list = server->working_workers;
    List *leisure_list = server->leisure_workers;
    work_task_t *task  = (work_task_t *)worker->task;
    Socket *socket     = (Socket *)worker->socket;
    int ret, len, len_bak;

    len = socket->recv(socket, task->buf, task->buf_len, 0);

    // len = 0, means connect close by peer; len = -1, means received a rst packet
    if (len == 0 || len == -1) {
        dbg_str(NET_DETAIL, "tcp server, remove worker, fd=%d, ret=%d", fd, ret);
        server->close_subsocket(server, worker->socket);
        //task->req   //???????
        return 1;
    } else if (len < 0) {
        dbg_str(NET_ERROR, "socket read error, ret = %d", len);
        perror("new_conn_ev_callback\n");
        exit(1);
    }

    if (worker->work_callback && len) {
        task->opaque  = server->opaque;
        len_bak       = task->buf_len;
        task->buf_len = len;
        task->socket  = socket;
        worker->work_callback(task);
        task->buf_len = len_bak;
    }

    return 0;
}

static Worker *__get_worker(Server *server)
{
    Worker *ret = NULL;
    allocator_t *allocator = server->obj.allocator;
    List *leisure_list = server->leisure_workers;
    work_task_t *task;

    leisure_list->remove(leisure_list, (void **)&ret);

    if (ret == NULL) {
        dbg_str(NET_DETAIL, "get_worker, alloc a new worker");
        ret = object_new(allocator, "Worker", NULL);

#define TASK_MAX_BUF_LEN 1024 * 10
        task = work_task_alloc(allocator, TASK_MAX_BUF_LEN);
        ret->task = (void *)task;
#undef TASK_MAX_BUF_LEN
    } else {
        dbg_str(NET_DETAIL, "get_worker, get worker from leisure list");
    }

    return ret;
}

static ssize_t __listenfd_ev_callback(int fd, short event, void *arg)
{
    Worker *worker         = (Worker *)arg;
    Server *server         = (Server *)worker->opaque;
    Socket *socket         = server->socket;
    Producer *producer     = global_get_default_producer();
    List *working_list     = server->working_workers;
    allocator_t *allocator = worker->obj.allocator;
    Worker *new_worker;
    Socket *new_socket;

    new_socket = socket->accept(socket, NULL, NULL);
    new_worker = __get_worker(server);
    if (new_worker == NULL) {
        dbg_str(NET_ERROR, "OBJECT_NEW Worker");
        return -1;
    }

    dbg_str(NET_VIP, "listenfd_ev_callback, new fd = %d, sizeof(worker)=%d, sizeof(socket)=%d",
            new_socket->fd, sizeof(Worker), sizeof(Socket));

    new_worker->opaque = server;
    new_worker->socket = new_socket;
    new_worker->assign(new_worker, new_socket->fd, EV_READ | EV_PERSIST, NULL, 
                       (void *)__new_conn_ev_callback, 
                       new_worker, 
                       (void *)worker->work_callback);
    new_worker->enroll(new_worker, producer);
    new_socket->opaque = new_worker;

    working_list->add(working_list, new_worker);

    return 0;
}

static int __trustee(Server *server, void *work_callback, void *opaque)
{
    Producer *producer = global_get_default_producer();
    Worker *worker     = server->worker;
    int fd             = server->socket->fd;
    Socket *socket     = server->socket;
    
    server->opaque = opaque;
    socket->setnonblocking(socket);
    socket->listen(socket, 1024);
    worker->opaque = server;
    worker->assign(worker, fd, EV_READ | EV_PERSIST, NULL, 
                   (void *)__listenfd_ev_callback, worker, work_callback);

    return worker->enroll(worker, producer);
}

static int __close_subsocket(Server *server, Socket *socket)
{
    Worker *worker;
    List *working_list = server->working_workers;
    List *leisure_list = server->leisure_workers;
    int ret;

    if (socket == NULL) return 0;
    worker = (Worker *)socket->opaque;

    ret = worker->resign(worker);
    if (ret == 1) {
        working_list->remove_element(working_list, worker);
        leisure_list->add(leisure_list, worker);
        object_destroy(socket);
        worker->socket = NULL;
        dbg_str(NET_DETAIL, "add worker %p to leisure_list", worker);
    } else {
        dbg_str(NET_WARNNING, "worker %p has resigned", worker);
        exit(-1);
    }

    return 1;
}

static class_info_entry_t concurent_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Server, construct, __construct),
    Init_Nfunc_Entry(2 , Server, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Server, set, NULL),
    Init_Vfunc_Entry(4 , Server, get, NULL),
    Init_Vfunc_Entry(5 , Server, bind, __bind),
    Init_Vfunc_Entry(6 , Server, trustee, __trustee),
    Init_Vfunc_Entry(7 , Server, close_subsocket, __close_subsocket),
    Init_End___Entry(8 , Server),
};
REGISTER_CLASS("Server", concurent_class_info);
