/**
 * @file Response.c
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
#include <stdlib.h>
#include <errno.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/try.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Rbtree_Map.h>
#include <libobject/core/String.h>
#include <libobject/net/http/Response.h>

static char *__trim(char *str)
{
    char *p = str;
    char *p1;

    if (p) {
        p1 = p + strlen(str) - 1;
        while (*p && isspace(*p)) p++;
        while (p1 > p && isspace(*p1)) *p1-- = '\0';
    }

    return p;
}

static int __construct(Response *response, char *init_str)
{
    allocator_t *allocator = response->obj.allocator;
    Map *map;

    response->chain  = object_new(allocator, "Object_Chain", NULL);
    response->buffer = object_new(allocator, "Ring_Buffer", NULL);
    response->body   = object_new(allocator, "Buffer", NULL);

    response->status_code = 0;

    map = object_new(allocator, "RBTree_Map", NULL);
    if (map != NULL) {
        response->headers = map;
    }
    map->set_cmp_func(map, string_key_cmp_func); 
    response->writable_flag = 1;

    return 0;
}

static int __deconstrcut(Response *response)
{
    if (response->file) {
        fclose(response->file);
    }
    object_destroy(response->chain);
    object_destroy(response->buffer);
    object_destroy(response->headers);
    object_destroy(response->body);

    return 0;
}

static int __set_status_code(Response *response, int status_code)
{
    response->status_code = status_code;

    return 0;
}

static int __get_status_code(Response *response)
{
    return response->status_code;
}

static int __set_body(Response *response, void *content, int content_len)
{
    Buffer *body;
    int ret = 0;

    body = response->body;

    body->set_capacity(body, content_len);
    ret = body->write(body, content, content_len);
    if (ret < 0) {
        return ret;
    }
    response->content_len = content_len;

    return 1;
}

static int __set_fmt_body(Response *response, int max_len, char *fmt, ...)
{
    allocator_t *allocator = response->obj.allocator;
    int ret = 0;
    va_list ap;
    char *buf_addr;
    Buffer *body;

    body = response->body;
    buf_addr = allocator_mem_alloc(allocator, max_len);
    if (buf_addr == NULL) {
        return -1;
    }
    memset(buf_addr, 0, max_len);

    va_start(ap, fmt);
    ret = vsnprintf(buf_addr, max_len, fmt, ap);
    va_end(ap);
    if (ret <= 0) {
        dbg_str(DBG_ERROR, "run at here");
        goto end;
    } else if (ret > max_len) {
        ret = -1;
        dbg_str(DBG_ERROR, "max_len par is too small!");
        goto end;
    }

    ret = body->write(body, buf_addr, strlen(buf_addr));
    if (ret <= 0) {
        dbg_str(DBG_ERROR, "run at here");
        goto end;
    }

    response->content_len += ret;

end:
    allocator_mem_free(allocator, buf_addr);

    return ret;
}

static char * __get_body(Response *response)
{
    return response->body->addr;
}

static int __set_header(Response *response, void *key, void *value)
{
    Object_Chain *chain = response->chain;
    String *k, *v;

    k = chain->new(chain, "String", key);
    v = chain->new(chain, "String", value);

    return response->headers->add(response->headers, 
                                  k->get_cstr(k), v->get_cstr(v));
}

static int __read_body(Response *response)
{
    Map *headers = response->headers;
    Ring_Buffer *rb = response->buffer;
    char *content_len_str;
    Buffer *body;
    int ret = 0, content_len, rb_len;

    response->status = STATUS_READ_BODY;
    content_len = response->content_len;

    rb_len = rb->get_len(rb);
    if (rb_len < 0) {
        ret = 0;
        goto end;
    }

    body = response->body;
    rb->read_to_buffer(rb, body, rb_len);

    if (body->get_len(body) == content_len) {
        ret = 1;
    } else {
        ret = 0;
    }

    dbg_str(NET_DETAIL, "body wrote len = %d, body capacity=%d, "
            "content_len =%d, want to wrote len=%d",
            body->get_len(body), body->capacity, content_len, rb_len);

end:
    return ret;
}

static int __read_headers(Response *response)
{
    Ring_Buffer *buffer = response->buffer;
    Object_Chain *chain = response->chain;
    Map *headers = response->headers;
    String *str;
    int len, cnt = 0, ret = 0;
    char *key, *value;

    dbg_str(NET_SUC, "read headers");
    response->status = STATUS_READ_HEADERS;
    str = chain->new(chain, "String", NULL);

    while (1) {
        len = buffer->get_needle_offset(buffer, "\r\n", 2);
        if (len < 0) {
            ret = -1;
            break;
        }

        if (str == NULL) break;
        str->reset(str);
        buffer->read_to_string(buffer, str, len + 2);

        if (str->equal(str, "\r\n") == 1) {
            ret = __read_body(response);
            break;
        }

        str->replace(str, "\r\n", "", -1);

        cnt = str->split(str, ":", 1);
        if (cnt != 2) {
            dbg_str(NET_ERROR, "response header format error");
            dbg_str(NET_ERROR, "header:%s", str->get_cstr(str));
            dbg_str(NET_ERROR, "split cnt=%d", cnt);
            ret = -1;
            break;
        } else if (cnt == 2) {
            key   = str->get_splited_cstr(str, 0);
            value = str->get_splited_cstr(str, 1);
            key   = __trim(key);
            value = __trim(value);
            dbg_str(NET_VIP, "key:%s, value:%s", key, value);

            response->set_header(response, key, value);
            if (strcmp(key, "Content-Length") == 0) {
                response->content_len = atoi(value);
                response->body->set_capacity(response->body, response->content_len);
            }
        }
    }

    return ret;
}

static int __read_start_line(Response *response)
{
    Ring_Buffer *buffer = response->buffer;
    Object_Chain *chain = response->chain;
    String *str = NULL;
    int len, cnt;
    char *version, *status, *description;

    response->status = STATUS_READ_START_LINE;

    len = buffer->get_needle_offset(buffer, "\r\n", 2);
    if (len < 0) {
        return -1;
    }

    str = chain->new(chain, "String", NULL);
    if (str == NULL) {
        return -1;
    }

    buffer->read_to_string(buffer, str, len + 2);

    str->replace(str, "\r\n", "", -1);

    cnt = str->split(str, " ", 2);
    if (cnt != 3) {
        dbg_str(NET_ERROR, "response start line format error");
        dbg_str(NET_ERROR, "split cnt=%d", cnt);
        return -1;
    } else {
        version     = str->get_splited_cstr(str, 0);
        status      = str->get_splited_cstr(str, 1);
        description = str->get_splited_cstr(str, 2);

        status = __trim(status);
        response->status_code = atoi(status);

        dbg_str(DBG_VIP, "version:%s, status:%d, description:%s",
                version, response->status_code, description);

        return __read_headers(response);
    }
}

static int __read(Response *response)
{
    int ret = 0;

    switch (response->status) {
        case STATUS_INIT:
            ret = __read_start_line(response);
            break;
        case STATUS_READ_START_LINE:
            ret = __read_start_line(response);
            break;
        case STATUS_READ_HEADERS:
            ret = __read_headers(response);
            break;
        case STATUS_READ_BODY:
            ret = __read_body(response);
            break;
        default:
            break;
    }

    return ret;
}


//need optimize....
static int __write_body(Response *response)
{
    int len, send_len = 0;
    int ret;
    Socket *socket;

    socket = response->socket;
    while (send_len < response->content_len) {
        len = response->content_len - send_len; 
        len = len > 2048 ? 2048 : len;
        ret = socket->send(socket, response->body->addr + send_len , len, 0);
        if (ret == -1) {
            dbg_str(DBG_ERROR, "http write file error, retry");
            usleep(1000);
            continue;
        } else if (ret < 0) {
            perror("http send file error\n");
            dbg_str(DBG_ERROR, "http write file error, error no:%d", ret);
            exit(1);
        }
        send_len += ret;
    }

    return 0;
}

//need optimize....
static int __write_file(Response *response)
{
    char buf[2048];
    int ret, cnt = 0, len, try_count = 0;
    Socket *socket;

    socket = response->socket;
    while (!feof(response->file)) {
        len = fread(buf, 1, sizeof(buf), response->file);
        cnt += len;

        while (len > 0) {
            if (try_count > 5) {
                return -1;
            }
            ret = socket->send(socket, buf, 2048, 0);
            if (ret == -1) {
                dbg_str(DBG_ERROR, "http write file error, retry again, error no:%d, strerror:%s",
                        ret, strerror(errno));
                return -1;
                /*
                 *try_count++;
                 *usleep(100000);
                 *continue;
                 */
            } else if (ret < -1) {
                dbg_str(DBG_ERROR, "http write file error, error no:%d, strerror:%s",
                        ret, strerror(errno));
                return -1;
            }

            try_count = 0;

            len -= ret;
        }
    }

    dbg_str(NET_SUC, "write file, file len=%d, cnt=%d", response->content_len, cnt);

    return 1;
}

static int __write_headers(Response *response)
{
    Ring_Buffer *b = response->buffer;
    Map *headers = response->headers;
    void *key, *value;
    Iterator *cur, *end;
    int len;
    char buffer[2048];
    int ret;
    Socket *socket;

    socket = response->socket;
    b->printf(b, "%s %d %s\r\n", "HTTP/1.1", response->status_code, "");

    cur = headers->begin(headers);
    end = headers->end(headers);
    for (; !end->equal(end, cur); cur->next(cur)) {
        key = cur->get_kpointer(cur);
        value = cur->get_vpointer(cur);
        b->printf(b, "%s: %s\r\n", key, value);
    }

    if (response->content_len != 0 || response->file != NULL) {
        b->printf(b, "Content-Length: %d\r\n", response->content_len);
        b->printf(b, "\r\n");
    } else {
        b->printf(b, "\r\n");
    }

    len = b->get_len(b);
    b->read(b, buffer, len);

    socket->send(socket, buffer, len, 0);

    return 0;
}

static int __write(Response *response)
{
    int ret;

    TRY {
        EXEC(__write_headers(response));

        if (response->file != NULL) {
            EXEC(__write_file(response));
        } else if (response->content_len != 0) {
            EXEC(__write_body(response));
        }
    } CATCH (ret) {
    }

    return ret;
}

static class_info_entry_t response_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Response, construct, __construct),
    Init_Nfunc_Entry(2 , Response, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Response, read, __read),
    Init_Vfunc_Entry(4 , Response, set_status_code, __set_status_code),
    Init_Vfunc_Entry(5 , Response, get_status_code, __get_status_code),
    Init_Vfunc_Entry(6 , Response, set_body, __set_body),
    Init_Vfunc_Entry(7 , Response, set_fmt_body, __set_fmt_body),
    Init_Vfunc_Entry(8 , Response, get_body, __get_body),
    Init_Vfunc_Entry(9 , Response, set_header, __set_header),
    Init_Vfunc_Entry(10, Response, write, __write),
    Init_End___Entry(11, Response),
};
REGISTER_CLASS("Response", response_class_info);
