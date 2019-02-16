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
#include <libobject/net/http/Request.h>

static void __http_request_init_internal(Request *);

static int __construct(Request *request,char *init_str)
{
    allocator_t *allocator = request->obj.allocator;
    request->request_header_context = OBJECT_NEW(allocator, String, NULL);
    request->port = OBJECT_NEW(allocator, String, NULL);
    request->content_len = 0;
    request->method = NULL;
    request->arguments = NULL;
    request->url = NULL;
    request->server_ip = NULL;
    request->version = NULL;
    request->port->assign(request->port,"8080");
    request->map =  OBJECT_NEW(allocator, RBTree_Map, NULL);
    request->request_cb = NULL;
    request->request_cb_arg = NULL;
    request->timer = NULL;
    request->ev_tv.tv_sec  = 2;
    request->ev_tv.tv_usec = 0;
    __http_request_init_internal(request);
    return 0;
}

static int __deconstrcut(Request *request)
{
    dbg_str(EV_DETAIL,"request deconstruct,request addr:%p",request);
    if (request->request_header_context)
        object_destroy(request->request_header_context);
    if (request->url)
        object_destroy(request->url);
    if (request->method)
        object_destroy(request->method);
    if (request->arguments)
        object_destroy(request->arguments);
    if (request->version)
        object_destroy(request->version);
    if (request->port)
        object_destroy(request->port);
    if (request->server_ip) 
        object_destroy(request->server_ip);
    if (request->timer) 
        timer_worker_destroy(request->timer);
    object_destroy(request->map);
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
    } else if (strcmp(attrib, "write") == 0) {
        request->write = value;
    } else if (strcmp(attrib, "option_valid") == 0) {
        request->option_valid = value;
    } else if (strcmp(attrib, "set_opt") == 0) {
        request->set_opt = value;
    } else if (strcmp(attrib, "option_reset") == 0) {
        request->option_reset = value;
    } else {
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

static  void __http_request_init_internal(Request *self)
{
    self->map->add(self->map,"Accept-Encoding","gzip, deflate");
    self->map->add(self->map,"Content-Type","text/html;charset=utf-8");
    self->map->add(self->map,"Accept","text/html");
    self->map->add(self->map,"Accept-Language","zh-CN");
    self->map->add(self->map,"Accept-Charset"," ISO-8859-1,utf-8");
    self->map->add(self->map,"Keep-Alive","300");
    self->map->add(self->map,"Connection","keep-alive");
    // self->map->add(self->map,"Content-Length","0");
    self->map->add(self->map,"Host","localhost");

}

static int __set_content_len(Request *request, int content_len)
{
    request->content_len = content_len;

    return 0;
}

static int __option_valid(Request *self,http_opt_t opt)
{
    if ((opt)==  HTTP_OPT_PROXY          ||
        (opt) == HTTP_OPT_EFFECTIVE_URL  ||
        (opt) == HTTP_OPT_USERAGENT      ||
        (opt) == HTTP_OPT_USERPWD        ||
        (opt) == HTTP_OPT_JSON_FORMAT    ||
        (opt) == HTTP_OPT_TIMEOUT        ||
        (opt) == HTTP_OPT_ENCODING       ||
        (opt) == HTTP_OPT_CONNECTTIMEOUT ||
        (opt) == HTTP_OPT_REFER          ||
        (opt) == HTTP_OPT_ACCEPT         ||
        (opt) == HTTP_OPT_SSLKEY         ||
        (opt) == HTTP_OPT_METHOD_POST    ||
        (opt) == HTTP_OPT_METHOD_GET     ||
        (opt) == HTTP_OPT_POSTFIELDS     ||
        (opt) == HTTP_OPT_ACCEPT_LAN     ||
        (opt) == HTTP_OPT_CONNTENT_TYPE  ||
        (opt) == HTTP_OPT_HOST           ||
        (opt) == HTTP_OPT_ACCEPT_CHARSET ||
        (opt) == HTTP_OPT_VERSION        ||
        (opt) == HTTP_OPT_METHOD         ||
        (opt) == HTTP_OPT_PORT           ||  
        (opt) == HTTP_OPT_CONNTENT_LEN   ||
        (opt) == HTTP_OPT_CALLBACK       ||
        (opt) == HTTP_OPT_CALLBACKDATA          
     ) {
         return 1;
     } else {
         return 0;
     }
}

static void __option_reset(Request *self)
{
    if (self->method != NULL) {
        self->method->clear(self->method);
    }

    if (self->version != NULL) {
        self->version->clear( self->version);
    }

    if (self->url != NULL) {
        self->url->clear( self->url);
    }

    if (self->arguments != NULL) {
        self->arguments->clear( self->arguments);
    }

    if (self->request_header_context != NULL ) {
      self->request_header_context->clear(self->request_header_context);  
    }
}

static int __request_set_header_method_internal(Request *self,void *value)
{    
    allocator_t * allocator = self->obj.allocator;
    if (self->method == NULL) {
        self->method = OBJECT_NEW(allocator,String,NULL);
    }
    if (value != NULL) {
        self->method->assign(self->method,(char *)value);
    } else {
        self->method->assign(self->method,"GET");
    }
    return 0;
}

static int __request_url_parse_internal(Request *self,String *url)
{
    int ret = 0;
    int pos;
    if (url == NULL || url->is_empty(url)) {
        ret = -1;
        goto end;
    }
    if (self->server_ip) {
        object_destroy(self->server_ip);
        self->server_ip = NULL;
    }
    if (self->url) {
        object_destroy(self->url);
        self->url = NULL;
    }

    url->trim(url);
    pos = url->find_cstr(url,"http://",0);
    if (pos == 0) {
        pos = url->find_cstr(url,"/",7);
        if (pos < 0) {
            self->server_ip = url->substr(url,7,url->size(url));
            if (!self->url) {
                self->url = OBJECT_NEW(self->obj.allocator,String,NULL);
            }    
            self->url->assign(self->url,"/");
        } else {
            self->server_ip = url->substr(url,7,pos-7);
            self->url = url->substr(url,pos,url->size(url));
        }
    } else if (pos < 0) {
        pos = url->find_cstr(url,"/",0);
        if (pos < 0) {
            self->server_ip = url->substr(url,0,url->size(url));
            if (!self->url) {
                self->url = OBJECT_NEW(self->obj.allocator,String,NULL);
            }    
            self->url->assign(self->url,"/");
        } else {
            self->server_ip = url->substr(url,0,pos);
            self->url = url->substr(url,pos,url->size(url));
        }
    } else {
        dbg_str(DBG_ERROR,"http request parse error");
        ret = -1;
        goto end;
    }
    end:
        return ret;
}

static int __request_set_header_url_internal(Request *self,void *value)
{   
    int ret = 0;
    allocator_t * allocator = self->obj.allocator;
    String *tmp = OBJECT_NEW(allocator,String,NULL);

    if (value == NULL) {
        ret = -1;
        goto end;
    }
    
    tmp->assign(tmp,(char *)value);
    ret = __request_url_parse_internal(self,tmp);
end:
        object_destroy(tmp);
        return ret;
}

static int __request_set_header_version_internal(Request *self,void *value)
{
    allocator_t * allocator = self->obj.allocator;
    if (self->version == NULL) {
        self->version = OBJECT_NEW(allocator,String,NULL);
    }
    if (value != NULL){
        self->version->assign(self->version,(char *)value);
    } else {
        self->version->assign(self->version,"HTTP/1.0");
    }
    return 0;
}

static void __convert_implent(void *key,void *element,void *arg)
{
    Request * self = arg;
    self->request_header_context->append_str(self->request_header_context,(char *)key);
    self->request_header_context->append_str(self->request_header_context,": ");
    self->request_header_context->append_str(self->request_header_context,(char *)element);
    self->request_header_context->append_str(self->request_header_context,"\r\n");
}

static void __convert_map_2_str_internal(Request *self)
{
    dbg_str(DBG_SUC,"self->map addr:%p %d ",self->map,self->map->size(self->map));
    ((Map *)self->map)->for_each_arg((Map *)self->map,__convert_implent,(void *)self);
}

static int __check_http_request_header(Request *self)
{   
    int ret = 0;
    if (self->url == NULL || self->url->is_empty(self->url)) {
        dbg_str(DBG_ERROR,"http request failed url is unvalid");
        if (!self->request_header_context->is_empty(self->request_header_context)){
            self->request_header_context->clear(self->request_header_context);
        }
        ret = -1;
        return ret;
    }

    if (self->method == NULL || self->method->is_empty(self->method)) {
        ret = __request_set_header_method_internal(self,NULL);
    }
    
    if (self->version == NULL || self->version->is_empty(self->version)) {
       ret= __request_set_header_version_internal(self,NULL);
    }

    __convert_map_2_str_internal(self);

    return ret;
}

static int __write(Request *self)
{
    int ret = 0;
    int pos = 0;
    allocator_t *allocator = self->obj.allocator;
    String * send_request;

    ret = __check_http_request_header(self);
    if (ret < 0) {
        return ret;
    }

    send_request = OBJECT_NEW(allocator,String,NULL);
    send_request->append_str(send_request,self->method->c_str(self->method));
    send_request->append_str(send_request," ");
    send_request->append_str(send_request,self->url->c_str(self->url));
    send_request->append_str(send_request," ");
    send_request->append_str(send_request,self->version->c_str(self->version));
    send_request->append_str(send_request,"\r\n");

    if (!self->request_header_context->is_empty(self->request_header_context)) {
        //send_request->append_str(send_request,self->request_header_context->c_str(self->request_header_context));
        self->request_header_context->insert_cstr_normal(self->request_header_context,
                                                         0,
                                                         send_request->c_str(send_request)
                                                         );  
    } else {
        self->request_header_context->assign(self->request_header_context,
                                             send_request->c_str(send_request)
                                            );
        self->request_header_context->append_str(self->request_header_context,"\r\n");
    }
    
    pos = self->request_header_context->find_cstr(self->request_header_context,
                                            "\r\n\r\n",
                                            self->request_header_context->size(self->request_header_context) -4
                                            );
    if (pos < 0) {
        self->request_header_context->append_str(self->request_header_context,"\r\n");
    }
    
    if (self->arguments != NULL && self->method->equal_ignore_cstr(self->method,"POST")) {
        self->request_header_context->append_str(self->request_header_context,
                                                 self->arguments->c_str(self->arguments)
                                                 );
    }

    send_request->clear(send_request);
    object_destroy(send_request);
    return ret;
}

static int __request_set_header_json_internal(Request *self,void *value)
{   
    int ret = 0; 
    allocator_t * allocator = self->obj.allocator;
    if (value == NULL) {
        ret = -1;
        return ret;
    }

    if (self->arguments == NULL) {
        self->arguments = OBJECT_NEW(allocator,String,NULL);
    }

    self->arguments->assign(self->arguments,(char *)value);
    
    return ret; 
}

static int __request_set_accept_internal(Request *self,void *value)
{
//
    int ret = 0; 
    void *e,*t;
    allocator_t * allocator = self->obj.allocator;
    self->map->remove(self->map,"Accept",(void **)&t);
    self->map->add(self->map,"Accept",value);
    return ret; 
}

static int __request_set_useragent_internal(Request *self,void *value)
{

}
static int __request_set_connecttimeout_internal(Request *self,void *value)
{

}

static int __request_set_refer_internal(Request *self,void *value)
{

}
static int __request_set_encoding_internal(Request *self,void *value)
{

}
static int __request_set_host_internal(Request *self,void *value)
{
    int ret = 0; 
    void *e,*t;
    allocator_t * allocator = self->obj.allocator;
    self->map->remove(self->map,"HOST",(void **)&t);
    self->map->add(self->map,"HOST",value);   
    return ret; 
}

static int __request_set_accept_lan_internal(Request *self,void *value)
{
    int ret = 0; 
    void *e,*t;
    allocator_t * allocator = self->obj.allocator;
    self->map->remove(self->map,"Accept-Language",(void **)&t);
    self->map->add(self->map,"Accept-Language",value);   
    return ret; 
}

static int __request_set_accept_contenttype_internal(Request *self,void *value)
{
    int ret = 0; 
    void *e,*t;
    allocator_t * allocator = self->obj.allocator;
    self->map->remove(self->map,"Content-Type",(void **)&t);
    self->map->add(self->map,"Content-Type",value);   
    return ret; 
}

static int __request_set_accept_content_len_internal(Request *self,void *value)
{
    int ret = 0; 
    void *e,*t;
    allocator_t * allocator = self->obj.allocator;
    self->map->remove(self->map,"Content-Length",(void **)&t);
    self->map->add(self->map,"Content-Length",value);   
    return ret; 
}

static int __request_set_accept_charset_internal(Request *self,void *value)
{
    int ret = 0; 
    void *e,*t;
    allocator_t * allocator = self->obj.allocator;
    self->map->remove(self->map,"Accept-Charset",(void **)&t);
    self->map->add(self->map,"Accept-Charset",value);   
    return ret; 
}

static int __request_set_header_port_internal(Request *self,void *value)
{
    int ret = 0; 
    allocator_t * allocator = self->obj.allocator;
    if (value == NULL) {
        ret = -1;
        return ret;
    }
    
    self->port->assign(self->port,(char *)value);
    return ret;
}

static int __request_set_callback_internal(Request *self,void *value)
{
    int ret = 0;
    if (value == NULL) {
        ret = -1;
        return ret;
    }

    self->request_cb = (request_cb_t)value;
    return ret;
}

static int __request_set_callbackdata_internal(Request *self,void *value)
{
    int ret = 0;
    if (value == NULL) {
        ret = -1;
        return ret;
    }
    self->request_cb_arg = value;
    return ret;
}

static int __request_set_timeout_internal(Request *self,void *value)
{
    if (!value) {
        return -1;
    }
    self->ev_tv.tv_sec  = *(int *)value;
    self->ev_tv.tv_usec = 0;

    return 0; 
}

static int __set_opt(Request *self,http_opt_t opt,void *value)
{   
    int ret = 0; 
    if (!self->option_valid(self,opt)) {
        return -1;
    }
    
    switch (opt)
    {
        case HTTP_OPT_ACCEPT:
            ret = __request_set_accept_internal(self,value);
            break;
        case HTTP_OPT_CONNECTTIMEOUT:
            ret = __request_set_connecttimeout_internal(self,value);
            break;
        case HTTP_OPT_ACCEPT_LAN:
            ret = __request_set_accept_lan_internal(self,value);
            break;
        case HTTP_OPT_CONNTENT_TYPE:
            ret = __request_set_accept_contenttype_internal(self,value);
            break;
        case HTTP_OPT_HOST:
            ret = __request_set_host_internal(self,value);
            break;
        case HTTP_OPT_ACCEPT_CHARSET:
            ret = __request_set_accept_charset_internal(self,value);
            break;
        case HTTP_OPT_METHOD_GET:
            ret = __request_set_header_method_internal(self,value);
            break;
        case HTTP_OPT_METHOD_POST:
            ret = __request_set_header_method_internal(self,value);
            break;
        case HTTP_OPT_EFFECTIVE_URL:
            ret = __request_set_header_url_internal(self,value);
            break;
        case HTTP_OPT_VERSION: 
            ret = __request_set_header_version_internal(self,value);
            break;
        case HTTP_OPT_JSON_FORMAT:
            ret = __request_set_header_json_internal(self,value);
            break;
         case HTTP_OPT_METHOD:
            ret = __request_set_header_method_internal(self,value);
            break;
        case HTTP_OPT_PORT:
            ret = __request_set_header_port_internal(self,value);
            break;
         case HTTP_OPT_CONNTENT_LEN:
            ret = __request_set_accept_content_len_internal(self,value);
            break;
        case HTTP_OPT_CALLBACK:
            ret = __request_set_callback_internal(self,value);
            break;
        case HTTP_OPT_CALLBACKDATA:
            ret = __request_set_callbackdata_internal(self,value);
            break;
        case HTTP_OPT_TIMEOUT:
            ret = __request_set_timeout_internal(self,value);
            break;
        default:
            break;
    }
    return ret;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","write",__write,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_opt",__set_opt,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","option_valid",__option_valid,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_content_len",__set_content_len,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","option_reset",__option_reset,sizeof(void *)},
    [10 ] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Request",concurent_class_info);

static int test_request()
{
    Request *request;
    allocator_t *allocator = allocator_get_default_alloc();
    int ret = 0; 
    request = OBJECT_NEW(allocator, Request, NULL);
    #if 0
        request->set_opt(request,HTTP_OPT_EFFECTIVE_URL,"http://ys-qf.cloutropy.com/mts_in/softlink/17c32c42e3bd68c407cd99906c09a99e.mkv");
        request->set_opt(request,HTTP_OPT_METHOD,"POST");
        request->set_opt(request,HTTP_OPT_JSON_FORMAT,"{json_data}");
        request->set_opt(request,HTTP_OPT_CONNTENT_TYPE,"application/json");
        request->set_opt(request,HTTP_OPT_ACCEPT,"plain/text1111");
        request->set_opt(request,HTTP_OPT_ACCEPT_CHARSET,"ISO-8000");
        //request->request_set_opt(request,REQUEST_OPT_CONNTENT_TYPE,"application/json");
    #else 
      //request->set_opt(request,HTTP_OPT_EFFECTIVE_URL,"http://www.baidu.com");
      request->set_opt(request,HTTP_OPT_EFFECTIVE_URL,"http://www.baidu.com/get_server_list");
      //request->set_opt(request,HTTP_OPT_METHOD,"GET");
    #endif

    request->write(request);
    if(!request->request_header_context->is_empty(request->request_header_context))
        dbg_str(DBG_SUC,"requst context header:\n%s",
                request->request_header_context->c_str(request->request_header_context));
    
    dbg_str(DBG_SUC,"server_ip:%s",request->server_ip->c_str(request->server_ip));
    dbg_str(DBG_SUC,"server_url:%s",request->url->c_str(request->url));

    request->option_reset(request);
    ret = request->write(request);
    if (ret < 0 ) {
        dbg_str(DBG_ERROR,"request error");
    }
    
    if(!request->request_header_context->is_empty(request->request_header_context))
        dbg_str(DBG_SUC,"requst context header:%s",
        request->request_header_context->c_str(request->request_header_context));

    object_destroy(request);
    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_request);
