/**
 * @file Http_Client.c
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
#include <libobject/net/http/Client.h>

static int __construct(Http_Client *client,char *init_str)
{
    allocator_t *allocator = client->obj.allocator;

    client->req = OBJECT_NEW(allocator, Request, NULL);
    client->host = "127.0.0.1";

    return 0;
}

static int __deconstruct(Http_Client *client)
{
    dbg_str(EV_DETAIL,"client deconstruct,client addr:%p",client);

    object_destroy(client->req);
    if (client->c != NULL) {
        client_destroy(client->c);
    }

    return 0;
}

static int __set(Http_Client *client, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        client->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        client->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        client->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        client->deconstruct = value;
    } else if (strcmp(attrib, "get_request") == 0) {
        client->get_request = value;
    } else if (strcmp(attrib, "request") == 0) {
        client->request = value;
    } else if (strcmp(attrib, "request_sync") == 0) {
        client->request_sync = value;
    } 
    else {
        dbg_str(EV_DETAIL,"client set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Http_Client *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"client get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static Request *__get_request(Http_Client *client)
{
    return client->req;
}

static int 
__request(Http_Client *hc, int (*request_cb)(void *arg), void *arg)
{
    allocator_t *allocator = hc->obj.allocator;

    if (hc->c != NULL) {
    } else {
        Client *c = NULL;
        char *str = "hello world";

        c = client(allocator, 
                   CLIENT_TYPE_INET_TCP, 
                   (char *)"127.0.0.1", //char *host, 
                   (char *)"1990", //char *client_port, 
                   request_cb, 
                   arg);

        client_connect(c, "127.0.0.1", "8080");

        client_send(c, str, strlen(str), 0);
        hc->c  = c;
    }

    return 0;
}

static Response * __request_sync(Http_Client *client)
{
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstruct,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_request",__get_request,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","request",__request,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","request_sync",__request_sync,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Http_Client",concurent_class_info);

static int test_request_callback(void *task)
{
    net_task_t *t = (net_task_t *)task;

    dbg_str(DBG_SUC,"request callback run, cabllback opaque=%p", t->opaque);
}
int test_http_client(TEST_ENTRY *entry)
{
    Http_Client *client;
    Request *req;
    Response *response;
    allocator_t *allocator = allocator_get_default_alloc();

    client = OBJECT_NEW(allocator, Http_Client, NULL);
    req = client->get_request(client);

    req->set_method(req, "GET");

    response = client->request(client, test_request_callback, client);

    pause();
    object_destroy(client);

    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_http_client);
