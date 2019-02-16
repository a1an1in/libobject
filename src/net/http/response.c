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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/net/http/Response.h>

static int __construct(Response *response,char *init_str)
{
    allocator_t *allocator = response->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL,"response construct, response addr:%p",response);
    response->buffer = OBJECT_NEW(response->obj.allocator,RingBuffer,NULL); 
    response->response_context = OBJECT_NEW(allocator,String,NULL);
    response->current_size  = 0;
    response->content_length = 0;
    return 0;
}

static int __deconstrcut(Response *response)
{
    dbg_str(DBG_DETAIL,"response deconstruct,response addr:%p",response);
    object_destroy(response->buffer);
    if (response->response_context) {
        object_destroy(response->response_context);
        response->response_context = NULL;
    }
    return 0;
}

static int __set(Response *response, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        response->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        response->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        response->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        response->deconstruct = value;
    } else if (strcmp(attrib, "set_buffer") == 0) {
        response->set_buffer = value;
    } else if (strcmp(attrib, "read") == 0) {
        response->read = value;
    } else if (strcmp(attrib, "response_parse") == 0) {
        response->response_parse = value;
    } 
    else {
        dbg_str(DBG_DETAIL,"response set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Response *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARNNING,"response get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __set_buffer(Response *response, RingBuffer *buffer)
{
    response->buffer = buffer;

    return 0;
}

static int __response_parse(Response *response,void *buffer,int len)
{
    if (len < 0 || buffer == NULL ) {
        dbg_str(DBG_ERROR,"http client recv failed! recv_len =%d buffer addr:%p ",len,buffer);
        return -1;
    }

    RingBuffer *rbuffer = response->buffer;
    int pos_start,pos_end;
    String *resp = response->response_context;
    String *tmp = NULL;

    if (len) {
        rbuffer->buffer_write(rbuffer,buffer,len);
    }

    if (!response->content_length) {
       resp->assign(resp,buffer);
       pos_start = resp->find_cstr(resp,"Content-Length:",0);

       pos_end = resp->find_cstr(resp,"\r\n",pos_start+15);
       tmp = resp->substr(resp,pos_start+15,pos_end-(pos_start+15));
       if (tmp == NULL) {  return -1;}
       tmp->trim(tmp);
       response->content_length = atoi(tmp->c_str(tmp));
       resp->clear(resp);
       tmp->clear(tmp);
       object_destroy(tmp);
       tmp = NULL;
    }

    response->current_size = rbuffer->buffer_used_size(rbuffer);
    return len;
}

static int __read(Response *response)
{
    dbg_str(DBG_SUC, "read response");
    return 1;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_buffer",__set_buffer,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","read",__read,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","response_parse",__response_parse,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Response",concurent_class_info);

int test_response()
{
    Response *response;
    allocator_t *allocator = allocator_get_default_alloc();

    response = OBJECT_NEW(allocator, Response, NULL);

    object_destroy(response);
}
REGISTER_STANDALONE_TEST_FUNC(test_response);
