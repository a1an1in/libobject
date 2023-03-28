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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/net/worker/Inet_Udp_Client.h>
#include <libobject/net/worker/Inet_Tcp_Client.h>
#include <libobject/net/worker/Inet_Tcp_Server.h>
#include <libobject/net/worker/api.h>

#define CLIENT_TYPE_INET_TCP "inet_tcp_client_type"
#define CLIENT_TYPE_INET_UDP "inet_udp_client_type"
#define CLIENT_TYPE_UNIX_TCP "unix_tcp_client_type"
#define CLIENT_TYPE_UNIX_UDP "unix_udp_client_type"

void *client(allocator_t *allocator, char *type,
             char *host, char *service)
{
    Client *client = NULL;

    //there may add id_str check....

    if (!strcmp(type,CLIENT_TYPE_INET_UDP)){
        client = OBJECT_NEW(allocator, Inet_Udp_Client, NULL);
        if (service != NULL)
            client->bind(client, host, service); 
    } else if (!strcmp(type,CLIENT_TYPE_INET_TCP)){
        client = OBJECT_NEW(allocator, Inet_Tcp_Client, NULL);
    } else {
        dbg_str(NET_WARNNING,"client error type");
        return NULL;
    }

    return (void *)client;
}

int client_connect(void *client, char *host, char *service)
{
    Client *c = (Client *)client;
    return c->connect(c, host, service);
}

int client_trustee(void *client, struct timeval *tv, void *work_callback, void *opaque)
{
    Client *c = (Client *)client;
    int ret;

    TRY {
        THROW_IF(work_callback == NULL, -1);
        c->trustee(c, NULL, (void *)work_callback, opaque);
    } CATCH (ret) {
    }

    return ret;
}

int client_send(void *client, void *buf, int len, int flags)
{
    Client *c = (Client *)client;
    return c->send(c, buf, len, flags);
}

int client_destroy(void *client)
{
    return object_destroy(client);
}

void *server(allocator_t *allocator, char *type,
             char *host, char *service,
             int (*process_task_cb)(void *arg),
             void *opaque)
{
    Server *server;

    if(!strcmp(type, SERVER_TYPE_INET_TCP)){
        server = OBJECT_NEW(allocator, Inet_Tcp_Server, NULL);
        server->bind(server, host, service); 
        if (process_task_cb != NULL)
            server->trustee(server, (void *)process_task_cb, opaque);
    } else {
        dbg_str(NET_WARNNING,"server type error");
        return NULL;
    }

    return (void *)server;
}

int server_destroy(void *server)
{
    return object_destroy(server);
}


static int test_work_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    dbg_str(NET_SUC,"%s", t->buf);
}

static int test_udp_client_recv(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Client *c = NULL;

    dbg_str(NET_DETAIL,"test_obj_client_recv");

    c = client(allocator, CLIENT_TYPE_INET_UDP,
               (char *)"127.0.0.1", (char *)"1989");
    client_trustee(c, NULL, test_work_callback, NULL);
#if (defined(WINDOWS_USER_MODE))
    system("pause"); 
#else
    pause();
#endif
    object_destroy(c);
}
REGISTER_TEST_CMD(test_udp_client_recv);

static int test_udp_client_send(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Client *c = NULL;
    char *str = "hello world";

    dbg_str(NET_DETAIL,"test_obj_client_send");

    c = client(allocator, CLIENT_TYPE_INET_UDP,
               (char *)"127.0.0.1", (char *)"1990");
    client_connect(c, "127.0.0.1", "1989");
    client_trustee(c, NULL, test_work_callback, NULL);
    client_send(c, str, strlen(str), 0);

    object_destroy(c);
    dbg_str(NET_WARNNING,"test_obj_client_send end");
}
REGISTER_TEST_CMD(test_udp_client_send);

static int test_inet_tcp_client(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Client *c = NULL;
    char *str = "hello world";

    dbg_str(NET_DETAIL, "test_obj_client_send");

    c = client(allocator, CLIENT_TYPE_INET_TCP, 
               (char *)"127.0.0.1", (char *)"19900");
    client_connect(c, "127.0.0.1", "11011");
    client_trustee(c, NULL, test_work_callback, NULL);
    client_send(c, str, strlen(str), 0);
    pause();

    object_destroy(c);
}
REGISTER_TEST_CMD(test_inet_tcp_client);

static int test_inet_tcp_server(TEST_ENTRY *entry, void *argc, void *argv)
{
    Server *s;
    allocator_t *allocator = allocator_get_default_instance();
    int pre_alloc_count, after_alloc_count;
    int ret;

    dbg_str(DBG_SUC, "test_inet_tcp_server");
    pre_alloc_count = allocator->alloc_count;
    s = (Server *)server(allocator, SERVER_TYPE_INET_TCP, 
                         "127.0.0.1", "11011",
                         test_work_callback, 0x1234);

#if (defined(WINDOWS_USER_MODE))
    system("pause");
#else
    pause();
#endif
    dbg_str(DBG_SUC, "run at here");
    server_destroy(s);

    after_alloc_count = allocator->alloc_count;
    ret = assert_equal(&pre_alloc_count, &after_alloc_count, sizeof(int));
    if (ret == 0) {
        dbg_str(NET_WARNNING,
                "server has memory omit, pre_alloc_count=%d, after_alloc_count=%d",
                pre_alloc_count, after_alloc_count);
        entry->ret = ret;
        return ret;
    }

    entry->ret = ret;

    return 1;
}
REGISTER_TEST_CMD(test_inet_tcp_server);
