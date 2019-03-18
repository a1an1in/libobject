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
#include <libobject/core/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/net/http/Client.h>

static int __http_client_response_callback(void *task);

static int __construct(Http_Client *client,char *init_str)
{
    allocator_t *allocator = client->obj.allocator;

    client->req = OBJECT_NEW(allocator, Request, NULL);
    client->resp = OBJECT_NEW(allocator, Response, NULL);
    client->host = "127.0.0.1";

    client->req_buffer  = OBJECT_NEW(allocator, Buffer, NULL);
    client->resp_buffer = OBJECT_NEW(allocator, Buffer, NULL);

    return 0;
}

static int __deconstruct(Http_Client *client)
{
    dbg_str(EV_DETAIL,"client deconstruct,client addr:%p",client);

    object_destroy(client->req_buffer);
    object_destroy(client->resp_buffer);
    object_destroy(client->req);
    object_destroy(client->resp);

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
    } else if (strcmp(attrib, "get_response") == 0) {
        client->get_response = value;
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

static Response *__get_response(Http_Client *client)
{
    return client->resp;
}

static int 
__request(Http_Client *hc, int (*request_cb)(void *arg), void *arg)
{
    allocator_t *allocator = hc->obj.allocator;
    Request *req;
    int len;

    if (hc->c != NULL) {
    } else {
        Client *c = NULL;
        char *str = "hello world";

        req = hc->get_request(hc);
        len = req->buffer->w_offset;

        hc->request_cb = request_cb;
        hc->request_cb_arg = arg;

        c = client(allocator, 
                   CLIENT_TYPE_INET_TCP, 
                   (char *)"127.0.0.1", //char *host, 
                   (char *)"19923", //char *client_port, 
                   __http_client_response_callback, 
                   arg);

        client_connect(c, "127.0.0.1", "8080");

        client_send(c, req->buffer->addr, len, 0);
        hc->c  = c;
    }

    return 0;
}

static Response * __request_sync(Http_Client *client)
{
}

static class_info_entry_t concurent_class_info[] = {
    Init_Obj___Entry(0, Obj, obj),
    Init_Nfunc_Entry(1, Http_Client, construct, __construct),
    Init_Nfunc_Entry(2, Http_Client, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Http_Client, set, NULL),
    Init_Vfunc_Entry(4, Http_Client, get, NULL),
    Init_Vfunc_Entry(5, Http_Client, get_request, __get_request),
    Init_Vfunc_Entry(6, Http_Client, get_response, __get_response),
    Init_Vfunc_Entry(7, Http_Client, request, __request),
    Init_Vfunc_Entry(8, Http_Client, request_sync, __request_sync),
    Init_End___Entry(9),
};
REGISTER_CLASS("Http_Client",concurent_class_info);

static int __http_client_response_callback(void *task)
{
    net_task_t *t = (net_task_t *)task;
    Http_Client *client = t->opaque;
    Response *resp = client->get_response(client); 
    int ret;

    dbg_str(DBG_SUC,"http client response callback run, cabllback opaque=%p", 
            t->opaque);

    resp->set_buffer(resp, client->resp_buffer);
    resp->buffer->memcopy(resp->buffer, t->buf, t->buf_len);
    ret = resp->read(resp);
    if (ret == 1) {
        client->request_cb(client->resp, client->request_cb_arg);
    }
}

static int test_request_callback(Response *resp, void *arg)
{
    dbg_str(DBG_SUC,"request callback run");
}

int test_http_client(TEST_ENTRY *entry)
{
    Http_Client *client;
    Request *req;
    Response *response;
    allocator_t *allocator = allocator_get_default_alloc();
    char *body = "hello world from libobject";

    client = OBJECT_NEW(allocator, Http_Client, NULL);
    req = client->get_request(client);

    req->set_method(req, "GET");
    req->set_uri(req, "/hello");
    req->set_http_version(req, "HTTP/1.1");
    req->set_header(req, "Host", "127.0.0.1:8080");
    req->set_header(req, "User-Agent", "libobject-http-client");
    req->set_buffer(req, client->req_buffer);
    req->set_body(req, body);
    req->set_content_len(req, strlen(body));
    req->write(req);

    dbg_str(DBG_SUC,"request opaque=%p",  client);
    response = client->request(client, test_request_callback, client);

    pause();
    object_destroy(client);

    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_http_client);
