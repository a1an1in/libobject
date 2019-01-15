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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/rbtree_map.h>
#include <libobject/net/http/Request.h>

static int __construct(Request *request,char *init_str)
{
    allocator_t *allocator = request->obj.allocator;


    request->header = OBJECT_NEW(allocator, RBTree_Map, NULL);
    request->content_len = 0;
    return 0;
}

static int __deconstrcut(Request *request)
{
    dbg_str(EV_DETAIL,"request deconstruct,request addr:%p",request);
    object_destroy(request->header);

    return 0;
}

static int __set(Request *request, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        request->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        request->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        request->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        request->deconstruct = value;
    } else if (strcmp(attrib, "set_method") == 0) {
        request->set_method = value;
    } else if (strcmp(attrib, "set_uri") == 0) {
        request->set_uri = value;
    } else if (strcmp(attrib, "set_http_version") == 0) {
        request->set_http_version = value;
    } else if (strcmp(attrib, "set_header") == 0) {
        request->set_header = value;
    } else if (strcmp(attrib, "set_body") == 0) {
        request->set_body = value;
    } else if (strcmp(attrib, "set_content_len") == 0) {
        request->set_content_len = value;
    } else if (strcmp(attrib, "set_buffer") == 0) {
        request->set_buffer = value;
    } else if (strcmp(attrib, "write") == 0) {
        request->write = value;
    } 
    else {
        dbg_str(EV_DETAIL,"request set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Request *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"request get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
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
    return request->header->add(request->header, key, value);
}

static int __set_body(Request *request, void *body)
{
    request->body = body;

    return 0;
}

static int __set_content_len(Request *request, int content_len)
{
    request->content_len = content_len;

    return 0;
}
static int __set_buffer(Request *request, Buffer *buffer)
{
    request->buffer = buffer;
    return 0;
}

int __write(Request *request)
{
    Buffer *b = request->buffer;
    Map *header = request->header;
    char *value;
    int ret;

    b->printf(b, "%s %s %s\r\n",
              request->method, 
              request->uri, 
              request->version);

    ret = header->search(header, "Host", (void **)&value);
    if (ret == 1) {
        b->printf(b, "Host: %s\r\n", (char *)value);
    }

    ret = header->search(header, "User-Agent", (void **)&value);
    if (ret == 1) {
        b->printf(b, "User-Agent: %s\r\n", (char *)value);
    }

    b->printf(b, "\r\n");

    if (request->content_len != 0) {
        b->memcopy(b, request->body, request->content_len);
    }

    dbg_str(DBG_DETAIL, "Request:%s", b->addr);
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_method",__set_method,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_uri",__set_uri,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_http_version",__set_http_version,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_header",__set_header,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_body",__set_body,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","set_content_len",__set_content_len,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","set_buffer",__set_buffer,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","write",__write,sizeof(void *)},
    [13] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Request",concurent_class_info);

int test_request()
{
    Request *request;
    allocator_t *allocator = allocator_get_default_alloc();

    request = OBJECT_NEW(allocator, Request, NULL);

    object_destroy(request);
}
REGISTER_STANDALONE_TEST_FUNC(test_request);
