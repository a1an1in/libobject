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

static int __construct(Server *server, char *init_str)
{
    allocator_t *allocator = server->obj.allocator;

    dbg_str(EV_DETAIL, "server construct, server addr:%p", server);
    server->workers = OBJECT_NEW(allocator, Linked_List, NULL);

    return 0;
}

static void __release_worker(void *element)
{
    Worker *worker = (Worker *)element;

    dbg_str(DBG_DETAIL, "release_worker, element: %p", element);
    worker->resign(worker);
    object_destroy(worker);
}

static int __deconstrcut(Server *server)
{
    List *list = server->workers;
    dbg_str(EV_DETAIL, "server deconstruct, server addr:%p", server);

    /*those socket are created when accepting a new connection*/
    list->for_each(list, __release_worker);
    object_destroy(server->workers);

    return 0;
}

static int __bind(Server *server, char *host, char *service)
{
    Socket *socket = server->socket;

    return socket->bind(socket, host, service);
}

static ssize_t __new_conn_ev_callback(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    Server *server = (Server *)worker->opaque;
    List *list     = server->workers;
#define EV_CALLBACK_MAX_BUF_LEN 1024 * 10
    char buf[EV_CALLBACK_MAX_BUF_LEN];
    int  buf_len = EV_CALLBACK_MAX_BUF_LEN, len = 0;
#undef EV_CALLBACK_MAX_BUF_LEN
    int ret;

    len = recv(fd, buf, buf_len, 0);

    if (len < 0) {
        dbg_str(DBG_ERROR, "socket read error");
        exit(1);
    }  else if (len == 0) {
        ret = worker->resign(worker);
        if (ret == 0) {
            list->remove_element(list, worker);
            object_destroy(worker); //????there may be prolem, worker event may havn't been reclaimed
            dbg_str(DBG_DETAIL, "client exit");
        }
        return 1;
    }

    dbg_str(DBG_SUC, "new_conn_ev_callback");
    if (worker->work_callback && len) {
        net_task_t *task;
        task = net_task_alloc(worker->obj.allocator, len);
        memcpy(task->buf, buf, len);
        task->opaque  = server->opaque;
        task->buf_len = len;
        task->fd = fd;
        worker->work_callback(task);
        net_task_free(task);
    }

    return 0;
}

static ssize_t __listenfd_ev_callback(int fd, short event, void *arg)
{
    Worker *worker         = (Worker *)arg;
    Server *server         = (Server *)worker->opaque;
    Socket *socket         = server->socket;
    allocator_t *allocator = worker->obj.allocator;
    Producer *producer     = global_get_default_producer();
    List *list             = server->workers;
    Worker *new_worker;
    int new_fd;

    new_fd = socket->accept_fd(socket, NULL, NULL);
    new_worker = OBJECT_NEW(allocator, Worker, NULL);
    if (new_worker == NULL) {
        dbg_str(DBG_ERROR, "OBJECT_NEW Worker");
        return -1;
    }

    new_worker->opaque = server;
    new_worker->assign(new_worker, new_fd, EV_READ | EV_PERSIST, NULL, 
                       (void *)__new_conn_ev_callback, 
                       new_worker, 
                       (void *)worker->work_callback);
    new_worker->enroll(new_worker, producer);

    list->add(list, new_worker);

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

static class_info_entry_t concurent_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Server, construct, __construct),
    Init_Nfunc_Entry(2 , Server, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Server, set, NULL),
    Init_Vfunc_Entry(4 , Server, get, NULL),
    Init_Vfunc_Entry(5 , Server, bind, __bind),
    Init_Vfunc_Entry(6 , Server, trustee, __trustee),
    Init_End___Entry(7 , Server),
};
REGISTER_CLASS("Server", concurent_class_info);
