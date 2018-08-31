/**
 * @file Inet_Tcp_Client.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-10-29
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
#include <libobject/net/client/inet_tcp_client.h>

static int __construct(Inet_Tcp_Client *client, char *init_str)
{
    allocator_t *allocator = client->parent.obj.allocator;

    dbg_str(NET_DETAIL, "Inet_Tcp_Client construct, client addr:%p", client);
    client->parent.socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);
    if (client->parent.socket == NULL) {
        dbg_str(DBG_ERROR, "OBJECT_NEW Inet_Udp_Socket");
        return -1;
    }

    client->parent.worker = OBJECT_NEW(allocator, Worker, NULL);
    if (client->parent.worker == NULL) {
        dbg_str(DBG_ERROR, "OBJECT_NEW Worker");
        return -1;
    }

    return 0;
}

static int __deconstrcut(Inet_Tcp_Client *client)
{
    dbg_str(NET_DETAIL, "Inet_Tcp_Client deconstruct, client addr:%p", client);
    object_destroy(client->parent.socket);
    object_destroy(client->parent.worker);

    return 0;
}

static int __set(Inet_Tcp_Client *client, char *attrib, void *value)
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

static void *__get(Inet_Tcp_Client *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(NET_WARNNING, "client get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }

    return NULL;
}


static class_info_entry_t inet_tcp_client_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Client", "parent", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "bind", NULL, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "connect", NULL, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "send", NULL, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "recv", NULL, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "trustee", NULL, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Inet_Tcp_Client", inet_tcp_client_class_info);

static int test_work_callback(void *task)
{
    net_task_t *t = (net_task_t *)task;
    dbg_str(DBG_SUC, "%s", t->buf);
}

#if 0
void test_obj_inet_tcp_client()
{
    Inet_Tcp_Client *client;
    allocator_t *allocator = allocator_get_default_alloc();
    char buf[2048];
    char *test_str = "hello world";

    dbg_str(DBG_DETAIL, "test_obj_inet_tcp_client_recv");
    client = OBJECT_NEW(allocator, Inet_Tcp_Client, NULL);
    /*
     *client->bind(client, "127.0.0.1", "11011"); 
     */
    client->connect(client, "127.0.0.1", "11011");
    client->trustee(client, NULL, test_work_callback, client);
    client->send(client, test_str, strlen(test_str), 0);

    pause();

    object_destroy(client);
}
#else
#endif
