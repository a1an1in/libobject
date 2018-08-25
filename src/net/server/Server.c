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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/net/server/server.h>
#include <libobject/core/linked_list.h>

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

static int __set(Server *server, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        server->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        server->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        server->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        server->deconstruct = value;
    }
    else if (strcmp(attrib, "bind") == 0) {
        server->bind = value;
    } else if (strcmp(attrib, "trustee") == 0) {
        server->trustee = value;
    } 
    else {
        dbg_str(EV_DETAIL, "server set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Server *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING, "server get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }

    return NULL;
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
    [0] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5] = {ENTRY_TYPE_VFUNC_POINTER, "", "bind", __bind, sizeof(void *)}, 
    [6] = {ENTRY_TYPE_VFUNC_POINTER, "", "trustee", __trustee, sizeof(void *)}, 
    [7] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Server", concurent_class_info);

/*
 *void test_obj_server()
 *{
 *    Server *server;
 *    allocator_t *allocator = allocator_get_default_alloc();
 *    configurator_t * c;
 *    char *set_str;
 *    cjson_t *root, *e, *s;
 *    char buf[2048];
 *
 *    c = cfg_alloc(allocator); 
 *    dbg_str(EV_SUC, "configurator_t addr:%p", c);
 *    cfg_config(c, "/Server", CJSON_STRING, "name", "alan server") ;  
 *
 *    server = OBJECT_NEW(allocator, Server, c->buf);
 *
 *    object_dump(server, "Server", buf, 2048);
 *    dbg_str(EV_DETAIL, "Server dump: %s", buf);
 *
 *    object_destroy(server);
 *    cfg_destroy(c);
 *}
 */
