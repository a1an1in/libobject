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
#include <stdlib.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/net/client/client.h>
#include <libobject/net/http/Response.h>
#include <libobject/net/http/Client.h>
#include <libobject/io/RingBuffer.h>

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

static int __set(Client *client, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        client->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        client->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        client->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        client->deconstruct = value;
    }
    else if (strcmp(attrib, "bind") == 0) {
        client->bind = value;
    } else if (strcmp(attrib, "connect") == 0) {
        client->connect = value;
    } else if (strcmp(attrib, "recv") == 0) {
        client->recv = value;
    } else if (strcmp(attrib, "send") == 0) {
        client->send = value;
    } else if (strcmp(attrib, "trustee") == 0) {
        client->trustee = value;
    } 
    else {
        dbg_str(NET_DETAIL, "client set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Client *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(NET_WARNNING, "client get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
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
    Http_Client * http_client = client->opaque;
    Response * response = http_client->resp;

    int ret  = 0;
    allocator_t *allocator = allocator_get_default_alloc();
    String * tmp = http_client->current_http_chunck;

#define EV_CALLBACK_MAX_BUF_LEN 1024 
    char buf[EV_CALLBACK_MAX_BUF_LEN] = {0};
    int  buf_len = EV_CALLBACK_MAX_BUF_LEN, len = 0;
#undef EV_CALLBACK_MAX_BUF_LEN

    if (fd != socket->fd)
            dbg_str(DBG_WARNNING,"fd != socket->fd");
   
    len = socket->recv(socket, buf, buf_len, 0);
    
    ret = response->response_parse(response,buf,len);
    if (ret < 0) {
        dbg_str(DBG_ERROR,"http response parse error socketfd =%d ",fd);
        worker->resign(worker);
        client_close(client);
        return 0;
    }
    
    if (response->current_size >= response->content_length) {
            worker->work_callback(client->opaque);
    }
    return 0;
}

static int __trustee(Client *client, struct timeval *tv, 
                     void *work_callback, void *opaque)
{
    Producer *producer = global_get_default_producer();
    Client *c          = (Client *)client;
    Worker *worker     = c->worker;
    int fd             = c->socket->fd;
    
    client->opaque = opaque;
    worker->opaque = client;
    worker->assign(worker, fd, EV_READ | EV_PERSIST, tv, 
                   (void *)__ev_callback, worker, work_callback);
    worker->enroll(worker, producer);

    return 0;
}

static class_info_entry_t client_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "bind", __bind, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "connect", __connect, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "send", __send, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "recv", __recv, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "trustee", __trustee, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Client", client_class_info);

void test_obj_client()
{
    Client *client;
    allocator_t *allocator = allocator_get_default_alloc();
    char buf[2048];

    dbg_str(DBG_DETAIL, "test_obj_client");
    client = OBJECT_NEW(allocator, Client, NULL);

    /*
     *object_dump(client, "Client", buf, 2048);
     *dbg_str(NET_DETAIL, "Client dump: %s", buf);
     */
    pause();

    object_destroy(client);
}
