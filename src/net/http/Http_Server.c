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
#include <sys/stat.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/net/http/Server.h>
#include <libobject/concurrent/net/Inet_Tcp_Server.h>


int (*handler_not_found)(Request *req, Response *res, void *opaque) = __handler_not_found;

static int __http_server_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    Http_Server *server = (Http_Server *)t->opaque;
    allocator_t *allocator = server->obj.allocator;
    Request *req;
    Response *response;
    int len = 0, ret, read_ret = 0;

    if (t->buf_len <= 0) {
        object_destroy(t->request);
        t->request = NULL;
        return 0;
    }

    if (t->request == NULL) {
        dbg_str(NET_SUC,"http request start");
        req = object_new(allocator, "Request", NULL);
        req->status = STATUS_INIT;
        req->server = server;
        t->request = req;
    } else {
        dbg_str(NET_SUC,"http request continue");
        req = t->request;
    }

    while (len != t->buf_len) {
        ret = req->buffer->write(req->buffer,
                                 t->buf + len, t->buf_len - len);
        if (ret >= 0) { len += ret; }

        dbg_str(NET_DETAIL,"task buffer len=%d, "
                "write ring buffer len=%d, total wrote len=%d",
                t->buf_len, ret, len);

        read_ret = req->read(req);
        if (read_ret == 1 || read_ret < 0) {
            break;
        }
    } 

    if (read_ret == 1) {
        req->socket = t->socket;
        server->process_request(server, req);
        object_destroy(req);
        t->request = NULL;

        dbg_str(NET_SUC,"http request end");
    } else if (read_ret < 0) {
        dbg_str(NET_ERROR,"http request error end, ret=%d", read_ret);
        object_destroy(req);
        t->request = NULL;
        //... response error
    }

    return 0;
}

static int __construct(Http_Server *hs,char *init_str)
{
    allocator_t *allocator = hs->obj.allocator;
    Map *map;
    int value_type = VALUE_TYPE_ALLOC_POINTER;
    int trustee_flag = 1;

    map = object_new(allocator, "RBTree_Map", NULL);
    map->set_cmp_func(map, string_key_cmp_func);
    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set(map, "/Map/value_type", &value_type);
    hs->get_handlers = map;

    map = object_new(allocator, "RBTree_Map", NULL);
    map->set_cmp_func(map, string_key_cmp_func);
    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set(map, "/Map/value_type", &value_type);
    hs->post_handlers = map;

    map = object_new(allocator, "RBTree_Map", NULL);
    map->set_cmp_func(map, string_key_cmp_func);
    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set(map, "/Map/value_type", &value_type);
    hs->other_handlers = map;

    return 0;
}

static int __deconstruct(Http_Server *server)
{
    server_destroy(server->s);
    
    if (server->get_handlers != NULL) {
        object_destroy(server->get_handlers);
    }

    if (server->post_handlers != NULL) {
        object_destroy(server->post_handlers);
    }

    if (server->other_handlers != NULL) {
        object_destroy(server->other_handlers);
    }

    if (server->host != NULL) {
        object_destroy(server->host);
    }

    if (server->service != NULL) {
        object_destroy(server->service);
    }

    if (server->root != NULL) {
        object_destroy(server->root);
    }

    return 0;
}

static int __process_static_request(Http_Server *server, Request *r, Response *resp)
{
    struct stat st;
    char filename[MAX_FILE_NAME_LEN];
    FILE *file = NULL;
    int ret = 1;

    TRY {
        dbg_str(NET_VIP, "process_static_request, method:%s, uri:%s", r->method, r->uri);

        snprintf(filename, MAX_FILE_NAME_LEN, "%s%s", 
                server->root->get_cstr(server->root),
                (char *)r->uri);

        EXEC(stat(filename, &st));

        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            __handler_get_directory_list(r, resp, server);
            server->response(server, r, resp);
            return 1;
        }

        dbg_str(NET_SUC, "request filename:%s", filename);

        file = fopen(filename, "r");
        THROW_IF(file == NULL, -1);
        resp->file = file;
        resp->content_len = st.st_size;
        resp->set_status_code(resp, 200);

        EXEC(server->response(server, r, resp));
    } CATCH (ret) {
        dbg_str(NET_ERROR, "process_static_request, filename:%s, ret:%d", filename, ret);
    }

    return ret;
}

static int __process_cgi_request(Http_Server *server, Request *r, Response *resp)
{
    dbg_str(NET_VIP, "process_cgi_request, method:%s, uri:%s", r->method, r->uri);
}

static int __process_dynamic_request(Http_Server *server, Request *r, Response *resp)
{
    Map *map = NULL;
    int (*handler_cb)(Request *, Response *, void *opaque) = NULL;
    handler_t *handler = NULL;

    dbg_str(NET_VIP, "process_dynamic_request, method:%s, uri:%s", r->method, r->uri);

    if (strcmp(r->method, "GET") == 0) {
        map = server->get_handlers;
    } else if (strcmp(r->method, "POST") == 0) {
        map = server->post_handlers;
    } else {
        map = server->other_handlers;
    }

    map->search(map, r->uri, (void **)&handler);

    if (handler) {
        handler_cb = handler->callback;
        handler_cb(r, resp, handler->opaque);
        server->response(server, r, resp);
        return 1;
    } else {
        dbg_str(NET_ERROR, "process_request, method %s, uri:%s is not supported!", r->method, r->uri);
        handler_not_found(r, resp, server);
        server->response(server, r, resp);
        return 1;
    }
}

static int __process_request_cookie(Http_Server *server, Request *r, Response *resp)
{
    Object_Chain *chain = r->chain;
    Map *headers = r->headers;
    String *str;
    char *value = NULL;
    int i, cnt, ret;

    TRY {
        headers->search(headers, "cookie", (void **)&value);
        THROW_IF(value == NULL, 0);
        r->cookies = chain->new(chain, "String", value);
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static int __process_request(Http_Server *server, Request *r)
{
    enum request_type_e type;
    Response *resp;
    int ret;

    TRY {
        resp = object_new(server->obj.allocator, "Response", NULL);
        resp->socket = r->socket;

        EXEC(__process_request_cookie(server, r, resp));

        type = r->get_request_type(r);
        SET_CATCH_INT_PARS(type, 0);
        switch (type) {
            case REQUEST_TYPE_STATIC:
                EXEC(__process_static_request(server, r, resp));
                break;
            case REQUEST_TYPE_CGI:
                EXEC(__process_cgi_request(server, r, resp));
                break;
            case REQUEST_TYPE_DYNAMIC:
                EXEC(__process_dynamic_request(server, r, resp));
                break;
            case REQUEST_TYPE_NOT_DEFINED:
                break;
            default:
                break;
        }

        r->response = (void *)resp; //response 的任务处理完，托管给request；
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}

static int __register_handler(Http_Server *server,
                              char *method, char *path,
                              int (*handler)(Request *, Response *, void *),
                              void *opaque)
{
    Map *map = NULL;
    handler_t *h = NULL;

    if (strcmp(method, "GET") == 0) {
        dbg_str(DBG_INFO, "register get handler:%s", path);
        map = server->get_handlers;
    } else if (strcmp(method, "POST") == 0) {
        dbg_str(DBG_INFO, "register post handler:%s", path);
        map = server->post_handlers;
    } else {
        map = server->other_handlers;
    }

    h = allocator_mem_alloc(server->obj.allocator, sizeof(handler_t));
    h->method = method;
    h->callback = handler;
    h->path = path;
    h->opaque = opaque;

    map->add(map, path, h);

    dbg_str(NET_DETAIL, "register_handler map addr:%p, map trustee_flag=%d, map count:%d, method:%s path:%s",
            map, map->trustee_flag, map->count(map), method, path);

    return 1;
}


/* 注意如果插件有注册handler， 插件unload前必须去登记， 否则销毁handler map是会出现访问不存在的地址从而导致段错误。 */
static int __deregister_handler(Http_Server *server, char *method, char *path)
{
    Map *map = NULL;
    handler_t *h = NULL;
    void *element = NULL;
    int ret;

    if (strcmp(method, "GET") == 0) {
        dbg_str(DBG_INFO, "deregister get handler:%s", path);
        map = server->get_handlers;
    } else if (strcmp(method, "POST") == 0) {
        dbg_str(DBG_INFO, "deregister post handler:%s", path);
        map = server->post_handlers;
    } else {
        map = server->other_handlers;
    }

    ret = map->remove(map, path, &element);
    if (ret < 0) {
        return -1;
    }

    allocator_mem_free(server->obj.allocator, element);

    return 1;
}

static int 
__override_inner_handler(Http_Server *server, char *key,
                         int (*handler)(Request *, Response *, void *))
{
    if (strcmp(key, "NOT_FOUND")) {
        handler_not_found = handler;
    }

    return 1;
}

static int 
__response(Http_Server *server, Request *req, Response *res)
{
    return res->write(res);
}

static int __start(Http_Server *http_server)
{
    allocator_t *allocator = http_server->obj.allocator;
    Server *s;

    dbg_str(DBG_DETAIL, "http server, host:%s, service:%s", 
            STR2A(http_server->host), STR2A(http_server->service));

    s = (Server *)server(allocator, SERVER_TYPE_INET_TCP,
                         STR2A(http_server->host), STR2A(http_server->service),
                         __http_server_callback, http_server);
    http_server->s = s;

    return 1;
}

static class_info_entry_t concurent_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Http_Server, construct, __construct),
    Init_Nfunc_Entry(2 , Http_Server, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Http_Server, set, NULL),
    Init_Vfunc_Entry(4 , Http_Server, get, NULL),
    Init_Vfunc_Entry(5 , Http_Server, process_request, __process_request),
    Init_Vfunc_Entry(6 , Http_Server, register_handler, __register_handler),
    Init_Vfunc_Entry(7 , Http_Server, override_inner_handler, __override_inner_handler),
    Init_Vfunc_Entry(8 , Http_Server, response, __response),
    Init_Vfunc_Entry(9 , Http_Server, start, __start),
    Init_Vfunc_Entry(10, Http_Server, deregister_handler, __deregister_handler),
    Init_Str___Entry(11, Http_Server, root, NULL),
    Init_Str___Entry(12, Http_Server, host, NULL),
    Init_Str___Entry(13, Http_Server, service, NULL),
    Init_End___Entry(14, Http_Server),
};
REGISTER_CLASS(Http_Server, concurent_class_info);
