/**
 * @file Request.c
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
#include <sys/types.h>
#include <errno.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Rbtree_Map.h>
#include <libobject/core/try.h>
#include <libobject/net/http/Request.h>
#include <libobject/net/http/Server.h>

static int __read_headers(Request *request);
static int __read_body(Request *request);

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

static int __to_lower(char *str)
{
    int size = strlen(str);
    int i;

    for (i = 0; i < size; i++) {
        if (isupper(str[i])) {
            str[i] += 'a'-'A';
        }
    } 

    return 1;
}

static Form_Data_Type __get_multipart_form_data_type(String *str)
{
    if (strstr(STR2A(str), "image") != NULL) {
        return E_FORM_DATA_TYPE_IMAGE;
    } else if (strstr(STR2A(str), "image") != NULL) {
        return E_FORM_DATA_TYPE_VIDEO;
    } else {
        return E_FORM_DATA_TYPE_UNKNOW;
    }
}


static int __construct(Request *request, char *init_str)
{
    allocator_t *allocator = request->obj.allocator;

    request->headers = object_new(allocator, "RBTree_Map", NULL);
    request->headers->set_cmp_func(request->headers, string_key_cmp_func); 

    request->forms = object_new(allocator, "RBTree_Map", NULL);
    request->forms->set_cmp_func(request->headers, string_key_cmp_func); 

    request->chain  = object_new(allocator, "Object_Chain", NULL);
    request->buffer = object_new(allocator, "Ring_Buffer", NULL);
    request->body   = object_new(allocator, "Buffer", NULL);

    request->content_len = 0;

    return 0;
}

static int __deconstruct(Request *request)
{
    Map *headers = request->headers;
    Http_Server *server = (Http_Server *)request->server;
    char *key, *value = NULL;

    object_destroy(request->response);

    headers->search(headers, "connection", (void **)&value);
    if (value != NULL &&(strcmp(value, "close") == 0)) {
        Server *s = server->s;
        dbg_str(DBG_ERROR, "found Connection header:%s", value);
        s->close_subsocket(s, request->socket);
        request->socket = NULL;
    }

    object_destroy(request->headers);
    object_destroy(request->forms);
    object_destroy(request->chain);
    object_destroy(request->buffer);
    object_destroy(request->body);

    return 0;
}

static int __set_method(Request *request, void *method)
{
    request->method = method;

    return 0;
}

static int __set_uri(Request *request, void *uri)
{
    request->uri = uri;

    return 0;
}

static int __set_http_version(Request *request, void *version)
{
    request->version = version;

    return 0;
}

static int __set_header(Request *request, void *key, void *value)
{
    return request->headers->add(request->headers, 
                                key, value);
}

static int __set_body(Request *request, void *content, int content_len)
{
    Buffer *body;
    int ret = 0;

    body = request->body;

    body->set_capacity(body, content_len);
    ret = body->write(body, content, content_len);
    if (ret < 0) {
        return ret;
    }
    request->content_len = content_len;

    return 0;
}

static char *__get_form_value(Request *request, char *form_name)
{
    char *ret;

    request->forms->search(request->forms, form_name, (void **)&ret);

    return ret;
}

static int __get_request_type(Request *request)
{
    struct stat st;
    Http_Server *server = (Http_Server *)request->server;
    char filename[MAX_FILE_LEN];

    snprintf(filename, MAX_FILE_LEN, "%s%s", 
             server->root->get_cstr(server->root),
             (char *)request->uri);

    if (stat(filename, &st) == -1) {
        return REQUEST_TYPE_DYNAMIC;
    }

    if ((st.st_mode & S_IFMT) == S_IFDIR || (st.st_mode & S_IFMT) == S_IFREG) {
        return REQUEST_TYPE_STATIC;
    }

    if (    (st.st_mode & S_IXUSR) ||
            (st.st_mode & S_IXGRP) ||
            (st.st_mode & S_IXOTH))
    {
        /*
         *return REQUEST_TYPE_CGI;
         */
        return REQUEST_TYPE_STATIC;
    }

    return REQUEST_TYPE_NOT_DEFINED;
}

static int __read_form(Request *request)
{
    Object_Chain *chain = request->chain;
    String *str = NULL;
    char *uri, *query;
    int i, cnt;

    str = chain->new(chain, "String", request->uri);
    if (str == NULL) return -1;

    cnt= str->split(str, "[?]", 1);
    if (cnt <= 1) {
        return 0;
    }

    request->uri = str->get_splited_cstr(str, 0);
    query = str->get_splited_cstr(str, 1);

    str = chain->new(chain, "String", query);
    if (str == NULL) {
        return -1;
    }

    cnt= str->split(str, (char *)"[=&]", -1);
    if (cnt == 1) {
        return 0;
    } else if (cnt % 2 == 1) {
        return -1;
    }
    
    for (i = 0; i < cnt; i += 2) {
        printf("form %d:%s\n", i, str->get_splited_cstr(str, i));
        request->forms->add(request->forms, 
                            str->get_splited_cstr(str, i),
                            str->get_splited_cstr(str, i + 1));
    }

    return 1;
}

static int __read_form_data(Request *request)
{
    Object_Chain *chain = request->chain;
    Buffer *buffer = request->body;
    Http_Server *server = (Http_Server *)request->server;
    char *dir_name[E_FORM_DATA_TYPE_UNKNOW] = { "image", "video" };
    char *regex = "filename=\"([a-z0-9A-Z_.,!&=-]+)\"";
    char filename[MAX_FILE_LEN] = {0};
    char path[MAX_FILE_LEN] = {0};
    int len, start = 0, ret = 1;
    String *str;
    File *file;
    Form_Data_Type form_data_type;

    TRY {
        file = chain->new(chain, "File", NULL);
        while (buffer->get_len(buffer) > 2) {
            len = buffer->get_needle_offset(buffer, "\r\n", 2);
            if (len < 0) break;

            str = chain->new(chain, "String", NULL);
            THROW_IF(str == NULL, -1);
            EXEC(buffer->read_to_string(buffer, str, len + 2));
            __to_lower(STR2A(str));

            if (str->equal(str, "\r\n") == 1) {
                len = buffer->get_needle_offset(buffer, "\r\n--", 4);
                snprintf(path + strlen(path), MAX_FILE_LEN, "/%s", filename);
                dbg_str(NET_SUC, "write file len:%d, path:%s", len, path);
                EXEC(file->open(file, path, "w+"));
                file->write(file, buffer->addr + buffer->r_offset, len);
                file->close(file);
                buffer->r_offset += (len + 4);
                request->file = file;
                continue;
            }
            str->replace(str, "\r\n", "", -1);

            if (strstr(STR2A(str), "content-disposition") != NULL) {
                EXEC(str->get_substring(str, regex, 0, &start, &len));
                THROW_IF(start > str->get_len(str), -1);
                str->value[start + len] = '\0';
                snprintf(filename, MAX_FILE_LEN, "%s", str->value + start);
                dbg_str(NET_SUC, "form data filename:%s", filename);
            } else if (strstr(STR2A(str), "content-type") != NULL) {
                dbg_str(NET_SUC, "Line:%s", STR2A(str));
                form_data_type = __get_multipart_form_data_type(str);
                THROW_IF(form_data_type == E_FORM_DATA_TYPE_UNKNOW, -1);
                snprintf(path, MAX_FILE_LEN, "%s/%s", STR2A(server->root), dir_name[form_data_type]);
                dbg_str(NET_SUC, "path:%s", path);
                ret = file->is_exist(file, path);
                if (ret == 0) {
                    EXEC(file->mkdir(file, path, 0777));
                }
            } else {
                dbg_str(NET_SUC, "ignore Line:%s", STR2A(str));
            }
        }
    } CATCH (ret) {
        dbg_str(NET_ERROR, "len:%d, filename:%s", len, filename);
        dbg_str(NET_ERROR, "str:%s", STR2A(str));
    }
    return ret;
}

static int __read_start_line(Request *request)
{
    Ring_Buffer *buffer = request->buffer;
    Object_Chain *chain = request->chain;
    String *str = NULL;
    int len, cnt, ret;
    char *version, *method, *url;

    request->status = STATUS_READ_START_LINE;

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
        dbg_str(NET_ERROR, "request start line format error");
        dbg_str(NET_ERROR, "split cnt=%d", cnt);
        return -1;
    }

    method  = str->get_splited_cstr(str, 0);
    url     = str->get_splited_cstr(str, 1);
    version = str->get_splited_cstr(str, 2);

    method  = __trim(method);
    url     = __trim(url);
    version = __trim(version);

    dbg_str(NET_DETAIL, "method:%s, url:%s, version:%s",
            method, url, version);

    request->method = method;
    request->uri = url;

    ret = __read_form(request);
    if (ret < 0) return ret;

    return __read_headers(request);
}

static int __read_headers(Request *request)
{
    Ring_Buffer *buffer = request->buffer;
    Object_Chain *chain = request->chain;
    Map *headers = request->headers;
    String *str;
    int len, cnt = 0, ret = 0;
    char *key, *value;

    request->status = STATUS_READ_HEADERS;

    while (1) {
        len = buffer->get_needle_offset(buffer, "\r\n", 2);
        if (len < 0) {
            ret = -1;
            break;
        }

        str = chain->new(chain, "String", NULL);
        if (str == NULL) { ret = -1; break; }
        buffer->read_to_string(buffer, str, len + 2);

        if (str->equal(str, "\r\n") == 1) {
            ret = __read_body(request);
            break;
        }

        str->replace(str, "\r\n", "", -1);

        cnt = str->split(str, ":", 1);
        if (cnt != 2) {
            dbg_str(NET_ERROR, "request headers format error");
            dbg_str(NET_ERROR, "split cnt=%d", cnt);
            ret = -1;
            break;
        } else if (cnt == 2) {
            key   = str->get_splited_cstr(str, 0);
            value = str->get_splited_cstr(str, 1);
            key   = __trim(key);
            value = __trim(value);
            __to_lower(key);
            dbg_str(NET_SUC, "key:%s, value:%s", key, value);

            headers->add(headers, key, value);
            if (strcmp(key, "content-length") == 0) {
                request->content_len = atoi(value);
                request->body->set_capacity(request->body, request->content_len);
            }
        }
    }

    return ret;
}

static int __is_body_multipart_form_data(Request *request)
{
    Map *headers = request->headers;
    char *value = NULL;
    int ret = 0;

    TRY {
        headers->search(headers, "content-type", (void **)&value);
        THROW_IF(value == NULL, 0);
        THROW_IF((strstr(value, "multipart/form-data") == NULL), 0);
        THROW(1);
    } CATCH (ret) {
    }

    return ret;
}

static int __read_body(Request *request)
{
    Map *headers = request->headers;
    Ring_Buffer *rb = request->buffer;
    char *content_len_str;
    Buffer *body;
    int ret = 1, content_len, rb_len;

    TRY {
        request->status = STATUS_READ_BODY;
        content_len = request->content_len;

        rb_len = rb->get_len(rb);
        THROW_IF(rb_len < 0, 0);

        body = request->body;
        rb->read_to_buffer(rb, body, rb_len);
        THROW_IF(body->get_len(body) != content_len, 0);

        dbg_str(NET_SUC, "body wrote len = %d, body capacity=%d, "
                "content_len =%d, want to wrote len=%d, ret = %d",
                body->get_len(body), body->capacity, content_len, rb_len, ret);

        THROW_IF(__is_body_multipart_form_data(request) == 0, 1); //如果没有form_data, 直接结束read_body
        EXEC(__read_form_data(request)); /* 如果有form_data需要解析 */
    } CATCH (ret) {
    }

    return ret;
}

static int __read(Request *request)
{
    enum http_status_e status;
    int ret = 0;

    switch (request->status) {
        case STATUS_INIT:
            ret = __read_start_line(request);
            break;
        case STATUS_READ_START_LINE:
            ret = __read_start_line(request);
            break;
        case STATUS_READ_HEADERS:
            ret = __read_headers(request);
            break;
        case STATUS_READ_BODY:
            ret = __read_body(request);
            break;
        default:
            break;
    }

    return ret;
}


static int __write_file(Request *request)
{
    return 0;
}

static int __write_body(Request *request)
{
    int len, send_len = 0;
    int ret;
    Socket *socket;

    socket = request->socket;
    while (send_len < request->content_len) {
        len = request->content_len - send_len; 
        len = len > 2048 ? 2048 : len;
        ret = socket->send(socket, request->body->addr + send_len , len, 0);
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

static int __write_headers(Request *request)
{
    Ring_Buffer *b = request->buffer;
    Map *headers = request->headers;
    void *key, *value;
    Iterator *cur, *end;
    int ret, len;
    Socket *socket;

    socket = request->socket;
    b->printf(b, "%s %s %s\r\n", request->method, 
              request->uri, request->version);

    cur = headers->begin(headers);
    end = headers->end(headers);
    for (; !end->equal(end, cur); cur->next(cur)) {
        key = cur->get_kpointer(cur);
        value = cur->get_vpointer(cur);
        b->printf(b, "%s: %s\r\n", key, value);
    }

    if (request->content_len != 0) {
        b->printf(b, "Content-Length: %d\r\n", request->content_len);
        b->printf(b, "\r\n");
    } else {
        b->printf(b, "\r\n");
    }

    len = b->get_len(b);
    ret = socket->send(socket, b->addr + b->r_offset, len, 0);
    if (ret < 0) {
        dbg_str(DBG_ERROR, "http send req header error, error no:%d, error str:%s",
                ret, strerror(errno));
    } else {
        b->r_offset += ret;
    }

    return ret;

}

static int __write(Request *request)
{
    int ret;

    ret = __write_headers(request);
    if (ret < 0) return ret;

    if (request->file != NULL) {
        ret = __write_file(request);
    } else if (request->content_len != 0) {
        ret = __write_body(request);
    }

    return ret;
}

static int __get_cookie(Request *request, char *key, char *value, int value_len)
{
    String *cookies = request->cookies;
    char regex[50] = {0};
    int ret = 1, start = -1, len;

    TRY {
        THROW_IF(cookies == NULL, 0);
        SET_CATCH_STR_PARS(key, NULL);
        snprintf(regex, 50, "%s=([a-z0-9A-Z\._,&=-]+);", key);
        EXEC(cookies->get_substring(cookies, regex, 0, &start, &len));
        THROW_IF(start == -1, 0);
        snprintf(value, value_len, "%.*s", len, &cookies->value[start]);

        return ret;
    } CATCH (ret) {
        TRY_SHOW_STR_PARS(DBG_ERROR);
    }
}

static class_info_entry_t request_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Request, construct, __construct),
    Init_Nfunc_Entry(2 , Request, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Request, set_method, __set_method),
    Init_Vfunc_Entry(4 , Request, set_uri, __set_uri),
    Init_Vfunc_Entry(5 , Request, set_http_version, __set_http_version),
    Init_Vfunc_Entry(6 , Request, set_header, __set_header),
    Init_Vfunc_Entry(7 , Request, set_body, __set_body),
    Init_Vfunc_Entry(8 , Request, write, __write),
    Init_Vfunc_Entry(9 , Request, read, __read),
    Init_Vfunc_Entry(10, Request, get_form_value, __get_form_value),
    Init_Vfunc_Entry(11, Request, get_request_type, __get_request_type),
    Init_Vfunc_Entry(12, Request, get_cookie, __get_cookie),
    Init_End___Entry(13, Request),
};
REGISTER_CLASS("Request", request_class_info);
