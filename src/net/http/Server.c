/**
 * @file Http_Server.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-01-01
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
#include <libobject/core/utils/registry/registry.h>
#include <libobject/net/http/Server.h>
#include <libobject/net/server/inet_tcp_server.h>

static int __http_server_callback(void *task)
{
    net_task_t *t = (net_task_t *)task;
    dbg_str(DBG_SUC,"%s", t->buf);
    dbg_str(DBG_SUC,"task opaque=%p", t->opaque);
}

static int __construct(Http_Server *hs,char *init_str)
{
    allocator_t *allocator = hs->obj.allocator;

    dbg_str(DBG_SUC,"worker callback opaque=%p", server);
    hs->s = (Server *)server(allocator, SERVER_TYPE_INET_TCP, 
                         "127.0.0.1", "8080", __http_server_callback, server);

    return 0;
}

static int __deconstruct(Http_Server *server)
{
    server_destroy(server->s);

    return 0;
}

static int __set(Http_Server *server, char *attrib, void *value)
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
    else {
        dbg_str(EV_DETAIL,"server set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Http_Server *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"server get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstruct,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Http_Server",concurent_class_info);

int test_http_server(TEST_ENTRY *entry)
{
    Http_Server *server;
    allocator_t *allocator = allocator_get_default_alloc();

    server = OBJECT_NEW(allocator, Http_Server, NULL);

    pause();
    object_destroy(server);
}
REGISTER_STANDALONE_TEST_FUNC(test_http_server);
