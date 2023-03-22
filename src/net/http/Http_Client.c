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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/net/http/Client.h>

static int __http_client_response_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    Http_Client *client = t->opaque;
    Response *resp = client->get_response(client); 
    int ret;
    int len = 0;

    dbg_str(NET_DETAIL,"http client response callback run, cabllback opaque=%p", 
            t->opaque);

    while (len != t->buf_len) {
        ret = resp->buffer->write(resp->buffer, t->buf + len, t->buf_len - len);
        if (ret >= 0) {
            len += ret;
            
            dbg_str(NET_DETAIL,"task buffer len=%d, write ring buffer len=%d, total wrote len=%d",
                    t->buf_len, ret, len);

        }

        ret = resp->read(resp);
        if (ret == 1) {
            client->request_cb(client->resp, client->request_cb_arg);
            break;
        } else if (ret < 0) {
            //error, while reading request ....
        }
    } 
}


static int __construct(Http_Client *client,char *init_str)
{
    allocator_t *allocator = client->obj.allocator;

    client->req = object_new(allocator, "Request", NULL);
    client->resp = object_new(allocator, "Response", NULL);
    client->resp->status = STATUS_INIT;
    client->host = "127.0.0.1";

    return 0;
}

static int __deconstruct(Http_Client *client)
{
    dbg_str(EV_DETAIL,"client deconstruct,client addr:%p",client);

    object_destroy(client->req);
    object_destroy(client->resp);

    if (client->c != NULL) {
        client_destroy(client->c);
    }

    return 0;
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
__request(Http_Client *hc, int (*request_cb)(void *, void *), void *arg)
{
    allocator_t *allocator = hc->obj.allocator;
    Request *req;
    char *remote_host;
    char *remote_service;
    int len, ret;

    if (hc->c != NULL) {
    } else {
        Client *c = NULL;

        req = hc->get_request(hc);
        len = req->buffer->get_len(req->buffer);

        hc->request_cb = request_cb;
        hc->request_cb_arg = arg;

        c = client(allocator, CLIENT_TYPE_INET_TCP, 
                   (char *)"127.0.0.1", (char *)"19924");

        dbg_str(NET_SUC,"remote_host:%s remote_service:%s",
                hc->remote_host, hc->remote_service);

        ret = client_connect(c, hc->remote_host, hc->remote_service);
        if (ret < 0) {
            dbg_str(NET_ERROR, "client connect error");
            goto end;
        }
        client_trustee(c, NULL, __http_client_response_callback, arg);
        dbg_str(DBG_SUC,"run at here");

        req->socket = c->socket;
        req->write(req);

end:
        hc->c = c;
    }

    return 0;
}

static Response * __request_sync(Http_Client *client)
{
}

static class_info_entry_t http_client_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Http_Client, construct, __construct),
    Init_Nfunc_Entry(2 , Http_Client, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Http_Client, set, NULL),
    Init_Vfunc_Entry(4 , Http_Client, get, NULL),
    Init_Vfunc_Entry(5 , Http_Client, get_request, __get_request),
    Init_Vfunc_Entry(6 , Http_Client, get_response, __get_response),
    Init_Vfunc_Entry(7 , Http_Client, request, __request),
    Init_Vfunc_Entry(8 , Http_Client, request_sync, __request_sync),
    Init_Point_Entry(9 , Http_Client, remote_host, NULL),
    Init_Point_Entry(10, Http_Client, remote_service, NULL),
    Init_OP____Entry(11, Http_Client, command, NULL),
    Init_End___Entry(12, Http_Client),
};
REGISTER_CLASS("Http_Client", http_client_class_info);

static int test_request_callback(Response *resp, void *arg)
{
    Http_Client *client = (Http_Client *)arg;

    dbg_str(NET_SUC,"request callback run, arg=%p", arg);
    dbg_str(NET_SUC,"status code =%d", resp->status_code);
    dbg_str(NET_SUC,"body =%s", resp->get_body(resp));
}

int test_http_client(TEST_ENTRY *entry)
{
    Http_Client *client;
    Request *req;
    Response *response;
    allocator_t *allocator = allocator_get_default_instance();
    char *body = "hello world from libobject";
    char *remote_host = "127.0.0.1";
    char *remote_service = "8081";
    char **p1, **p2;

    client = OBJECT_NEW(allocator, Http_Client, NULL);
    req = client->get_request(client);
    client->set(client, "remote_host", remote_host);
    client->set(client, "remote_service", remote_service);

    dbg_str(NET_SUC,"get addr:%p",client->get);
    dbg_str(NET_SUC,"remote_host addr:%p",client->remote_host);
    p1 = client->get(client, "remote_host");
    p2 = client->get(client, "remote_service");

    dbg_str(NET_SUC,"remote_host:%s remote_service:%s",
            *p1, *p2);

    req->set_method(req, "GET");
    req->set_uri(req, "/hello");
    req->set_http_version(req, "HTTP/1.1");
    req->set_header(req, "Host", "127.0.0.1:8081");
    req->set_header(req, "User-Agent", "libobject-http-client");
    req->set_body(req, body, strlen(body));

    dbg_str(NET_SUC,"request opaque=%p",  client);
    response = client->request(client,
                               (int (*)(void *, void *))test_request_callback,
                               client);

    pause();
    object_destroy(client);

    return 1;
}
REGISTER_TEST_CMD(test_http_client);

