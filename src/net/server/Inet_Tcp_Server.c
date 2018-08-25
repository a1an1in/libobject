/**
 * @file Inet_Tcp_Server.c
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
#include <libobject/net/server/inet_tcp_server.h>

static int __construct(Inet_Tcp_Server *server, char *init_str)
{
    allocator_t *allocator = server->parent.obj.allocator;
    int opt = 1;

    dbg_str(EV_DETAIL, "Inet_Tcp_Server construct, server addr:%p", server);
    server->parent.socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);
    if (server->parent.socket == NULL) {
        dbg_str(DBG_ERROR, "OBJECT_NEW Inet_Udp_Socket");
        return -1;
    }
    server->parent.socket->setsockopt(server->parent.socket, 
                                      SOL_SOCKET, SO_REUSEADDR, 
                                      (void *)&opt);

    server->parent.worker = OBJECT_NEW(allocator, Worker, NULL);
    if (server->parent.worker == NULL) {
        dbg_str(DBG_ERROR, "OBJECT_NEW Worker");
        return -1;
    }

    return 0;
}

static int __deconstrcut(Inet_Tcp_Server *server)
{
    dbg_str(EV_DETAIL, "server deconstruct, server addr:%p", server);
    object_destroy(server->parent.socket);
    object_destroy(server->parent.worker);

    return 0;
}

static int __set(Inet_Tcp_Server *server, char *attrib, void *value)
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

static void *__get(Inet_Tcp_Server *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING, "server get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t concurent_class_info[] = {
    [0] = {ENTRY_TYPE_OBJ, "Server", "parent", NULL, sizeof(void *)}, 
    [1] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5] = {ENTRY_TYPE_IFUNC_POINTER, "", "bind", NULL, sizeof(void *)}, 
    [6] = {ENTRY_TYPE_IFUNC_POINTER, "", "trustee", NULL, sizeof(void *)}, 
    [7] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Inet_Tcp_Server", concurent_class_info);

static void test_work_callback(void *task)
{
    net_task_t *t = (net_task_t *)task;
    dbg_str(DBG_SUC, "%s", t->buf);
}

void test_obj_inet_tcp_server()
{
    Inet_Tcp_Server *server;
    allocator_t *allocator = allocator_get_default_alloc();

    /*
     *sleep(1);
     */
    dbg_str(DBG_DETAIL, "Inet_Tcp_Server=%d Inet_Tcp_Socket=%d worker=%d", 
             sizeof(Inet_Tcp_Server), sizeof(Inet_Tcp_Socket), sizeof(Worker));
    server = OBJECT_NEW(allocator, Inet_Tcp_Server, NULL);
    server->bind(server, "127.0.0.1", "11011"); 
    server->trustee(server, test_work_callback, NULL);

    pause();

    object_destroy(server);
}
