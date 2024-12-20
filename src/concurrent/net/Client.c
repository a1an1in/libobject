/**
 * @file Client.c
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
#include <libobject/core/utils/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/net/Client.h>

static int __construct(Client *client, char *init_str)
{
    dbg_str(NET_DETAIL, "client construct, client addr:%p", client);

    return 0;
}

static int __deconstrcut(Client *client)
{
    dbg_str(NET_DETAIL, "client deconstruct, client addr:%p", client);

    return 0;
}

static int __bind(Client *client, char *host, char *service)
{
    Socket *socket = client->socket;

    return socket->bind(socket, host, service);
}

static int __connect(Client *client, char *host, char *service)
{
    Socket *socket = client->socket;

    return socket->connect(socket, host, service);
}

static ssize_t __send(Client *client, const void *buf, size_t len, int flags)
{
    Socket *socket = client->socket;

    return socket->send(socket, buf, len, flags);
}

static ssize_t __recv(Client *client, void *buf, size_t len, int flags)
{
    Socket *socket = client->socket;

    return socket->recv(socket, buf, len, flags);
}

static ssize_t __ev_callback(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    Client *client = (Client *)worker->opaque;
    Socket *socket = client->socket;
    char buf[WORKER_TASK_MAX_BUF_LEN];
    int  buf_len = sizeof(buf), len = 0;
    work_task_t *task = worker->task;
    int ret;

    TRY {
        if (fd == socket->fd)
            len = socket->recv(socket, buf, buf_len, 0);

        if (len <= 0) {
            dbg_str(DBG_ERROR, "client ev_callback error, fd:%d, len=%d", fd, len);
        }

        if (worker->work_callback != NULL) {
            if (len > 0) memcpy(task->buf, buf, len);
            task->buf_len = len;
            task->fd = fd;
            task->opaque  = client->opaque;
            worker->work_callback(task);
        }
    } CATCH (ret) { }

    return ret;
}

static int __trustee(Client *client, struct timeval *tv, 
                     void *work_callback, void *opaque)
{
    Producer *producer = producer_get_default_instance();
    Client *c          = (Client *)client;
    Worker *worker     = c->worker;
    Socket *socket     = c->socket;
    int fd             = c->socket->fd;
    
    client->opaque = opaque;
    worker->opaque = client;
    worker->assign(worker, fd, EV_READ | EV_PERSIST, tv, 
                   (void *)__ev_callback, worker, work_callback);
    worker->enroll(worker, producer);
    //socket->setkeepalive(socket, 1, 60, 30, 3);

    return 0;
}

static class_info_entry_t client_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Client, construct, __construct),
    Init_Nfunc_Entry(2 , Client, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Client, set, NULL),
    Init_Vfunc_Entry(4 , Client, get, NULL),
    Init_Vfunc_Entry(5 , Client, bind, __bind),
    Init_Vfunc_Entry(6 , Client, connect, __connect),
    Init_Vfunc_Entry(7 , Client, send, __send),
    Init_Vfunc_Entry(8 , Client, recv, __recv),
    Init_Vfunc_Entry(9 , Client, trustee, __trustee),
    Init_End___Entry(10, Client),
};
REGISTER_CLASS(Client, client_class_info);
