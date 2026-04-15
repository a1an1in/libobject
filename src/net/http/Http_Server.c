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
#include <errno.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/net/http/Server.h>
#include <libobject/concurrent/net/Inet_Tcp_Server.h>

int (*handler_not_found)(Request *req, Response *res, void *opaque) = __handler_not_found;

static int __http_write_file(Request *req, Response *res)
{
    char buf[1024 * 8]; /* 8KB buffer */
    int ret = 0, len;
    Socket *socket;
    int full_flag = 0;

    TRY {
        THROW_IF(res == NULL || res->file == NULL, -1);

        /* Keep writing until socket buffer is full or file is complete */
        while (!feof(res->file) && !full_flag && (res->file_bytes_written < res->content_len)) {
            /* Read a chunk from file */
            len = fread(buf, 1, sizeof(buf), res->file);
            if (len <= 0) {
                THROW_IF(feof(res->file), 1); //The file has been read.
                dbg_str(DBG_ERROR, "file read error");
                THROW(-1);
            }

            /* Try to send the chunk */
            socket = res->socket;
            ret = socket->send(socket, buf, len, 0);
            if (ret == -1) {
                /* EAGAIN or EWOULDBLOCK - socket buffer is full */
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    /* Rewind file pointer back since we didn't send anything */
                    fseek(res->file, -len, SEEK_CUR);
                    full_flag = 1;
                    THROW(0);
                }
            } else if (ret == 0) {
                dbg_str(DBG_ERROR, "connection closed by peer during file write");
            } else if (ret < 0) {
                dbg_str(DBG_ERROR, "socket->send returned %d, error: %s", ret, strerror(errno));
            }
            THROW_IF(ret <= 0, -1);

            /* Successfully sent some data */
            res->file_bytes_written += ret;
            /* Handle partial write */
            if (ret < len) {
                fseek(res->file, -(len - ret), SEEK_CUR);
                dbg_str(DBG_VIP, "socket fd %d write partially, sent %d of %d bytes, total %d/%d, progress:%.1f%%",
                        socket->fd, ret, len, res->file_bytes_written, res->content_len, res->file_bytes_written * 100.0 / res->content_len);
                continue;
            }
        }

        if (res->file_bytes_written == res->content_len) {
            dbg_str(DBG_VIP, "socket fd %d write compeleted, sent %d of %d bytes, total %d/%d, progress:%.1f%%",
                    socket->fd, ret, len, res->file_bytes_written, res->content_len, res->file_bytes_written * 100.0 / res->content_len);
        }
        /* Check if file write is complete */
        THROW_IF(res->file_bytes_written >= res->content_len || feof(res->file), 1);
        /* More data to write, but socket buffer is full - normal case, not an error */
        THROW(0);
    } CATCH (ret) { } FINALLY {
        /* Clean up resources based on return value */
        if (ret == 1 || ret == -1) {
            /* File write completed (either successfully or with error) */
            if (res->file != NULL) {
                fclose(res->file);
                res->file = NULL;
            }
        }
        /* If ret == 0, we're not done yet, keep file open for next write */
    }

    return ret;
}

static int __http_work_for_write_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    Worker *worker = (Worker *)t->worker;
    Request *req;
    Response *res;
    int ret = 0;

    TRY {
        dbg_str(DBG_DETAIL, "tcp subsocket ev callback, fd:%d is writeable now, writing file.", t->fd);
        req = t->cache;
        THROW_IF(req == NULL, 0);

        res = (Response *)req->response;
        THROW_IF(res == NULL || res->file == NULL, -1);

        /* Try to write file data */
        EXEC(ret = __http_write_file(req, res));
        THROW(ret);
    } CATCH (ret) { } FINALLY {
        if (ret == 1 || ret < 0) {
            worker->adjust(worker, EV_READ | EV_PERSIST);
        }
    }

    return ret;
}

static int __http_work_for_read_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    Http_Server *server = (Http_Server *)t->opaque;
    allocator_t *allocator = server->obj.allocator;
    Request *req;
    int len = 0, ret = 0, read_ret = 0;

    if (t->buf_len <= 0) {
        object_destroy(t->cache);
        t->cache = NULL;
        return 0;
    }

    if (t->cache == NULL) {
        dbg_str(NET_SUC,"http request start");
        req = object_new(allocator, "Request", NULL);
        req->status = STATUS_INIT;
        req->server = server;
        t->cache = req;
    } else {
        dbg_str(NET_SUC,"http request continue");
        req = t->cache;
    }
    req->worker = t->worker;

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
        ret = server->process_request(server, req);
        dbg_str(NET_SUC,"http request end");
    } else if (read_ret < 0) {
        dbg_str(NET_ERROR,"http request error end, ret=%d", read_ret);
    }

    if (read_ret < 0 || (read_ret == 1 && ret != 206)) {
        object_destroy(req);
        t->cache = NULL;
    }

    return 0;
}

static int __http_work_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;

    if (t->event == EV_READ) {
        __http_work_for_read_callback(task);
    } else if (t->event == EV_WRITE) {
        __http_work_for_write_callback(task);
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
    char *range_header = NULL;
    handler_t *mp4_handler = NULL;

    TRY {
        dbg_str(NET_VIP, "process_static_request, method:%s, uri:%s", r->method, r->uri);
        snprintf(filename, MAX_FILE_NAME_LEN, "%s%s",
                server->root->get_cstr(server->root),
                (char *)r->uri);
        dbg_str(NET_SUC, "request filename:%s", filename);

        EXEC(stat(filename, &st));
        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            __handler_get_directory_list(r, resp, server);
            server->response(server, r, resp);
            return 1;
        }

        r->headers->search(r->headers, "range", (void **)&range_header);
        if (range_header != NULL && strstr(range_header, "bytes=") != NULL) { // 检查是否有Range头
            // 查找MP4 Range handler
            server->get_handlers->search(server->get_handlers,
                                        "range_handler",
                                        (void **)&mp4_handler);
            if (mp4_handler != NULL && mp4_handler->callback != NULL) {
                dbg_str(NET_VIP, "Calling range handler for file: %s", filename);
                return mp4_handler->callback(r, resp, mp4_handler->opaque);
            } else {
                resp->set_status_code(resp, 501);
                dbg_str(NET_WARN, "Range handler not found, falling back to normal file transfer");
                EXEC(server->response(server, r, resp));
                THROW(-1);
            }
        }

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
        if (r->response == NULL) {
            resp = object_new(server->obj.allocator, "Response", NULL);
            r->response = (void *)resp; //response 的任务处理完，托管给request；
        } else {
            resp = r->response;
        }
        
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

        THROW_IF(resp->get_status_code(resp) == 206, 206);

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
    Worker * worker = (Worker *)req->worker;

    if (res->file != NULL) {
        worker->adjust(worker, EV_READ | EV_WRITE | EV_PERSIST);
    }
    sleep(1);

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
                         __http_work_callback, http_server);
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
