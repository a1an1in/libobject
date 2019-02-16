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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/net/client/client.h>
#include <libobject/net/client/inet_udp_client.h>
#include <libobject/net/client/inet_tcp_client.h>

#if 1
#define CLIENT_TYPE_INET_TCP "inet_tcp_client_type"
#define CLIENT_TYPE_INET_UDP "inet_udp_client_type"
#define CLIENT_TYPE_UNIX_TCP "unix_tcp_client_type"
#define CLIENT_TYPE_UNIX_UDP "unix_udp_client_type"

void *client(allocator_t *allocator,
             char *type,
             char *host,
             char *service,
             int (*process_task_cb)(void *arg),
             void *opaque)
{
    Client *client = NULL;

    //there may add id_str check....

    if (!strcmp(type,CLIENT_TYPE_INET_UDP)){
        client = OBJECT_NEW(allocator, Inet_Udp_Client, NULL);
        client->bind(client, host, service); 
        if (process_task_cb != NULL)
            client->trustee(client, NULL, (void *)process_task_cb, opaque);
    } else if (!strcmp(type,CLIENT_TYPE_INET_TCP)){
        client = OBJECT_NEW(allocator, Inet_Tcp_Client, NULL);
        if (process_task_cb != NULL)
            client->trustee(client, NULL, (void *)process_task_cb, opaque);
    } else {
        dbg_str(DBG_WARNNING,"client error type");
        return NULL;
    }

    return (void *)client;
}

int client_connect(void *client, char *host, char *service)
{
    Client *c = (Client *)client;
    return c->connect(c, host, service);
}

int client_nonblock(void *client)
{
    Client *c  = (Client *)client;
    Socket * s = c->socket;
    return s->setnonblocking(s);
}

int client_close(void *client)
{
    Client *c  = (Client *)client;
    Socket * s = c->socket;
    return s->close(s); 
}

int client_send(void *client, void *buf, int len, int flags)
{
    Client *c = (Client *)client;
    return c->send(c, buf, len, flags);
}

int client_recv(void *client,void *buf,int len,int flags)
{
    Client *c = (Client *)client;
    return c->recv(c, buf, len, flags);
}

int client_destroy(void *client)
{
    return object_destroy(client);
}

static int test_work_callback(void *task)
{
    net_task_t *t = (net_task_t *)task;
    dbg_str(DBG_SUC,"%s", t->buf);
}

static int test_obj_client_recv(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_alloc();
    Client *c = NULL;

    dbg_str(DBG_DETAIL,"test_obj_client_recv");

    c = client(allocator,
               CLIENT_TYPE_INET_UDP,
               (char *)"127.0.0.1",//char *host,
               (char *)"1989",//char *client_port,
               test_work_callback,
               NULL);
    pause();
    object_destroy(c);
}
REGISTER_STANDALONE_TEST_FUNC(test_obj_client_recv);

static int test_obj_client_send(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_alloc();
    Client *c = NULL;
    char *str = "hello world";

    dbg_str(DBG_DETAIL,"test_obj_client_send");

    c = client(allocator,
               CLIENT_TYPE_INET_UDP,
               (char *)"127.0.0.1",//char *host,
               (char *)"1990",//char *client_port,
               test_work_callback,
               NULL);
    client_connect(c, "127.0.0.1", "1989");
    client_send(c, str, strlen(str), 0);

    pause();
    object_destroy(c);
}
REGISTER_STANDALONE_TEST_FUNC(test_obj_client_send);

static int test_obj_inet_tcp_client(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_alloc();
    Client *c = NULL;
    char *str = "hello world";

    dbg_str(DBG_DETAIL, "test_obj_client_send");

    c = client(allocator, 
               CLIENT_TYPE_INET_TCP, 
               (char *)"127.0.0.1", //char *host, 
               (char *)"1990", //char *client_port, 
               test_work_callback, 
               NULL);
    client_connect(c, "127.0.0.1", "11011");
    client_send(c, str, strlen(str), 0);

    pause();
    object_destroy(c);
}
REGISTER_STANDALONE_TEST_FUNC(test_obj_inet_tcp_client);

#endif
