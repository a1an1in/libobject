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

static int __http_client_response_callback(void *task);

static void __default_timeout_callback(void *);

static int __construct(Http_Client *client,char *init_str)
{
    allocator_t *allocator = client->obj.allocator;

    client->req = OBJECT_NEW(allocator, Request, NULL);
    client->resp = OBJECT_NEW(allocator, Response, NULL);
    client->host = "127.0.0.1";

    // client->req_buffer  = OBJECT_NEW(allocator, RingBuffer, NULL);
    // client->resp_buffer = OBJECT_NEW(allocator, RingBuffer, NULL);
    client->current_http_chunck = OBJECT_NEW(allocator, String, NULL);
    return 0;
}

static int __deconstruct(Http_Client *client)
{
    dbg_str(EV_DETAIL,"client deconstruct,client addr:%p",client);

    object_destroy(client->req);
    object_destroy(client->resp);
    object_destroy(client->current_http_chunck);
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
    } else if (strcmp(attrib, "set_opt") == 0) {
        client->set_opt = value;
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
__request(Http_Client *hc)
{
    allocator_t *allocator = hc->obj.allocator;
    Request *req;
    int ret,len;
    char *host ,*port ,*request_ctx = NULL;
    
    if (hc->c != NULL) {
    } else {
        Client *c = NULL;
        char *str = "hello world";

        req = hc->get_request(hc);
        ret = req->write(req);

        if (ret < 0 ) {
            return ret;
        }

        host = req->server_ip->c_str(req->server_ip);
        port = req->port->c_str(req->port);
        request_ctx = req->request_header_context->c_str(req->request_header_context);
        len = req->request_header_context->size(req->request_header_context) ;

        //timeout_callback
        req->timer = timer_worker(allocator,&req->ev_tv,hc,__default_timeout_callback);
 
        if (req->request_cb && req->request_cb_arg) {
            c = client(allocator, 
                       CLIENT_TYPE_INET_TCP, 
                       (char *)"127.0.0.1", //char *host, 
                       (char *)"19923", //char *client_port, 
                       req->request_cb, 
                       req->request_cb_arg);

        } else if (req->request_cb && !req->request_cb_arg) {
            c = client(allocator, 
                       CLIENT_TYPE_INET_TCP, 
                       (char *)"127.0.0.1", //char *host, 
                       (char *)"19923", //char *client_port, 
                       req->request_cb, 
                       hc);
        } else {
             c = client(allocator, 
                        CLIENT_TYPE_INET_TCP, 
                        (char *)"127.0.0.1", //char *host, 
                        (char *)"19923", //char *client_port, 
                        __http_client_response_callback, 
                        hc);
        }

        ret = client_connect(c,host,port);
        if (ret < 0) {
            dbg_str(DBG_ERROR,"http connect server(%s,%s) failed",host,port);
            req->option_reset(req);
            client_close(c);
            return ret;
        }
        
        client_send(c,
                    request_ctx,
                    len, 
                    0);
        req->option_reset(req);

        hc->c  = c;
    }

    return 0;
}

static Response * __request_sync(Http_Client *client)
{
}

static int __set_opt(Http_Client *client,http_opt_t opt,void *value)
{
    Request * req = NULL;
    req = client->get_request(client);
    if (req == NULL) {
        return -1;
    }
    
    return req->set_opt(req,opt,value);
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstruct,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_request",__get_request,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","get_response",__get_response,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","request",__request,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","request_sync",__request_sync,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_opt",__set_opt,sizeof(void *)},
    [10 ] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Http_Client",concurent_class_info);

static int __http_client_response_callback(void *arg)
{
    int ret = 0 ;
    dbg_str(DBG_SUC,"default request callback run recv");
    char buf[4096] = {0};
    Http_Client * client = arg;
    RingBuffer * ring_buf = client->resp->buffer;
    ring_buf->buffer_read(ring_buf,buf,4096);

    dbg_str(DBG_SUC,"request callback run recv %s",buf);
    return ret; 
}

static void __default_timeout_callback(void *arg)
{
    Http_Client * client = (Http_Client *)arg;
    Worker *worker = client->c->worker;
    
    dbg_str(DBG_ERROR," http request failed  timeout");
    client_close(client->c);
}

static int test_request_callback(void *arg)
{   
    int len = 0;
    dbg_str(DBG_SUC,"request callback run recv");
    char buf[4096] = {0};
    Http_Client * client = arg;
    Client *c = client->c;

    RingBuffer * ring_buf = client->resp->buffer;
    ring_buf->buffer_read(ring_buf,buf,4096);

    dbg_str(DBG_SUC,"request callback run recv %s",buf);
    Request *req = client->req;

    c->worker->resign(c->worker);
    client_close(c);

    return len; 
} 

int test_http_client(TEST_ENTRY *entry)
{
    int ret = 0 ;
    Http_Client *client;
    Response *response;
    int t = 4;
    allocator_t *allocator = allocator_get_default_alloc();
    char *body = "hello world from libobject";

    client = OBJECT_NEW(allocator, Http_Client, NULL);

    client->set_opt(client,HTTP_OPT_METHOD,"GET");
    client->set_opt(client,HTTP_OPT_EFFECTIVE_URL,"127.0.0.1");
    #if 1
    client->set_opt(client,HTTP_OPT_CALLBACK,test_request_callback);
    client->set_opt(client,HTTP_OPT_CALLBACKDATA,client);
    client->set_opt(client,HTTP_OPT_TIMEOUT,&t);
    #endif 

    #if 1
    ret = client->request(client);
    if ( ret < 0 ) {
        dbg_str(DBG_ERROR,"http request error!");
    }  
    #endif

    pause();
    
    object_destroy(client);
    
    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_http_client);
