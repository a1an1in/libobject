/**
 * @file bus.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 
 * @date 2016-10-28
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
#include <unistd.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/utils/blob/blob.h>
#include <libobject/bus/bus.h>
#include <libobject/utils/miscellany/buffer.h>
#include <libobject/utils/config/config.h>
#include <libobject/core/hash_map.h>

static const blob_policy_t bus_policy[] = {
    [BUS_ID]            = { .name = "id",              .type = BLOB_TYPE_INT32 },
    [BUS_OBJNAME]       = { .name = "object_name",     .type = BLOB_TYPE_STRING },
    [BUS_METHORDS]      = { .name = "methods",         .type = BLOB_TYPE_TABLE },
    [BUS_STATE]         = { .name = "state",           .type = BLOB_TYPE_INT32 },
    [BUS_OPAQUE]        = { .name = "opaque",          .type = BLOB_TYPE_BUFFER },
    [BUS_INVOKE_SRC_FD] = { .name = "source_fd",       .type = BLOB_TYPE_INT32 },
    [BUS_INVOKE_DST_FD] = { .name = "destination_fd",  .type = BLOB_TYPE_INT32 },
    [BUS_INVOKE_METHOD] = { .name = "invoke_method",   .type = BLOB_TYPE_STRING },
    [BUS_INVOKE_ARGC]   = { .name = "invoke_argc",     .type = BLOB_TYPE_INT32 },
    [BUS_INVOKE_ARGS]   = { .name = "invoke_args",     .type = BLOB_TYPE_TABLE },
};

bus_t * bus_alloc(allocator_t *allocator)
{
    bus_t *b;

    b = (bus_t *)allocator_mem_alloc(allocator,sizeof(bus_t));
    if ( b == NULL) {
        dbg_str(BUS_DETAIL,"allocator_mem_alloc");
        return NULL;
    }

    memset(b,0, sizeof(bus_t));

    b->allocator = allocator;

    return b;
}

int bus_set(bus_t *bus,char *attrib_name, char *value, int value_len)
{
    if (!strcmp(attrib_name,"client_sk_type")) {
        bus->client_sk_type = value;
    }else{
        dbg_str(BUS_WARNNING,"not support attrib setting,please check");
    }

    return 0;
}

int bus_init(bus_t *bus,
             char *server_host,
             char *server_srv,
             int (*process_client_task_cb)(void *task))
{
    configurator_t * c;

    if (bus->client_sk_type == NULL) {
        bus->client_sk_type = (char *)(&(CLIENT_TYPE_INET_TCP));
    }

    bus->server_host = server_host;
    bus->server_srv  = server_srv;

    bus->client = client(bus->allocator,
                         bus->client_sk_type,
                         bus->server_host,
                         bus->server_srv,
                         process_client_task_cb,
                         bus);

    client_connect(bus->client, bus->server_host, bus->server_srv);

    bus->blob = blob_create(bus->allocator);
    if (bus->blob == NULL) {
        client_destroy(bus->client);
        dbg_str(BUS_WARNNING,"blob_create");
        return -1;
    }
    blob_init(bus->blob);

    /*create object hash map*/
    c = cfg_alloc(bus->allocator); 
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "key_size", "40") ;  
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "value_size", "8") ;
    cfg_config(c, "/Hash_Map", CJSON_NUMBER, "bucket_size", "10") ;
    bus->obj_map = OBJECT_NEW(bus->allocator, Hash_Map,c->buf);
    bus->req_map = OBJECT_NEW(bus->allocator, Hash_Map,c->buf);

    cfg_destroy(c);
    return 1;
}

int bus_send(bus_t *bus,
             void *buf,
             size_t buf_len)
{
    int ret = 0;

    ret = client_send(bus->client, buf, buf_len, 0);
    if (ret < 0) {
        dbg_str(BUS_WARNNING,"bus send err");
    }

    return ret;
}

int bus_push_args_to_blob(blob_t *blob,struct bus_method *method)
{
    int i;

    for (i = 0; i < method->n_policy; i++) {
        blob_add_u32(blob, method->policy[i].name, method->policy[i].type);
    }

    return 0;
}

int bus_push_methods_to_blob(blob_t *blob,struct bus_object *obj)
{
    int i;

    dbg_str(BUS_DETAIL,"bus_push_method_to_blob,n_methods=%d",obj->n_methods);

    for (i = 0; i < obj->n_methods; i++) {
        dbg_str(BUS_DETAIL,"push method:%s",obj->methods[i].name);
        blob_add_table_start(blob, obj->methods[i].name);
        bus_push_args_to_blob(blob,&(obj->methods[i]));
        blob_add_table_end(blob);
        dbg_str(BUS_DETAIL,"push method end");
    }

    dbg_str(BUS_DETAIL,"bus_push_methods_to_blob end");

    return 0;
}

int __bus_add_obj(bus_t *bus,struct bus_object *obj)
{
    Map *map = bus->obj_map;

    return map->add(map, obj->name, obj);
}

int bus_add_object(bus_t *bus,struct bus_object *obj)
{
    bus_reqhdr_t hdr;
    blob_t *blob = bus->blob;
#define BUS_ADD_OBJECT_MAX_BUFFER_LEN 1024
    uint8_t buffer[BUS_ADD_OBJECT_MAX_BUFFER_LEN];
#undef BUS_ADD_OBJECT_MAX_BUFFER_LEN 
    uint32_t buffer_len;

    dbg_str(BUS_DETAIL,"bus_add_object,obj addr:%p",obj);
    memset(&hdr,0,sizeof(hdr));

    hdr.type = BUS_REQ_ADD_OBJECT;

    blob_add_table_start(blob,(char *)"object"); {
        blob_add_string(blob, (char *)"object_name", obj->name);
        blob_add_u32(blob, (char *)"id", 1);
        blob_add_table_start(blob, (char *)"methods"); {
            bus_push_methods_to_blob(blob, obj);
        }
        blob_add_table_end(blob);
    }
    blob_add_table_end(blob);

    /*
     *dbg_str(BUS_DETAIL,"run at here");
     */
    memcpy(buffer,&hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);
    /*
     *dbg_buf(BUS_DETAIL,"object:",blob->head,blob_get_len(blob->head));
     */
    memcpy(buffer + buffer_len,(uint8_t *)blob->head,blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    dbg_buf(BUS_DETAIL,"bus send:",buffer,buffer_len);
    bus_send(bus, buffer, buffer_len);

    __bus_add_obj(bus,obj);

    return 0;
}

int bus_handle_add_object_reply(bus_t *bus,  blob_attr_t **attr)
{
    int state;

    dbg_str(BUS_DETAIL,"bus_handle_add_object_reply");

    if (attr[BUS_STATE]) {
        state = blob_get_u32(attr[BUS_STATE]);
        dbg_str(BUS_DETAIL,"state=%d",state);
    }

    if (attr[BUS_OBJNAME]) {
        dbg_str(BUS_DETAIL,"object name:%s",blob_get_string(attr[BUS_OBJNAME]));
    }

    if ( state == 1) {
        dbg_str(BUS_SUC,"add obj success");
    } else {
        dbg_str(BUS_ERROR,"add obj failed");
        //..... del the obj
    }

    return 0;
}

int bus_lookup(bus_t *bus, char *key)
{
    bus_reqhdr_t hdr;
    blob_t *blob = bus->blob;
#define BUS_ADD_OBJECT_MAX_BUFFER_LEN 1024
    uint8_t buffer[BUS_ADD_OBJECT_MAX_BUFFER_LEN];
#undef BUS_ADD_OBJECT_MAX_BUFFER_LEN 
    uint32_t buffer_len;

    memset(&hdr,0,sizeof(hdr));

    hdr.type = BUS_REQ_LOOKUP;

    blob_add_table_start(blob,(char *)"lookup"); {
        blob_add_string(blob, (char *)"object_name", key);
    }
    blob_add_table_end(blob);

    /*
     *dbg_str(BUS_DETAIL,"run at here");
     */
    memcpy(buffer,&hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);
    /*
     *dbg_buf(BUS_DETAIL,"object:",blob->head,blob_get_len(blob->head));
     */
    memcpy(buffer + buffer_len,(uint8_t *)blob->head,blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    dbg_buf(BUS_DETAIL,"bus send:",buffer,buffer_len);
    bus_send(bus, buffer, buffer_len);

    return 0;
}

int bus_handle_lookup_object_reply(bus_t *bus,  blob_attr_t **attr)
{
    struct bus_object *obj;
    blob_attr_t *attrib,*head;
    uint32_t len;
    int ret;

    dbg_str(BUS_DETAIL,"bus_handle_lookup_object_reply");

    if (attr[BUS_ID]) {
        dbg_str(BUS_DETAIL,"object id:%d",blob_get_u32(attr[BUS_ID]));
    }
    if (attr[BUS_OBJNAME]) {
        dbg_str(BUS_DETAIL,"object name:%s",blob_get_string(attr[BUS_OBJNAME]));
    }
    if (attr[BUS_METHORDS]) {
        dbg_str(BUS_DETAIL,"object methods");

        head = (blob_attr_t *)blob_get_data(attr[BUS_METHORDS]);
        len  = blob_get_data_len(attr[BUS_METHORDS]);

        blob_for_each_attr(attrib, head, len) {
            dbg_str(BUS_DETAIL,"method name:%s",blob_get_name(attrib));
        }
    }
}

int bus_blob_add_args(blob_t *blob,int argc, bus_method_args_t *args)
{
    int i;

    for (i = 0; i < argc; i++) {
        if (args[i].type == ARG_TYPE_STRING) {
            blob_add_string(blob, (char *)args[i].name, args[i].value);
        } else if (args[i].type == ARG_TYPE_INT32) {
            blob_add_u32(blob, (char *)args[i].name, atoi(args[i].value));
        } else {
            dbg_str(BUS_WARNNING,"bus_blob_add_args,not support type = %d",args[i].type);
        }
        dbg_str(BUS_DETAIL,"bus_blob_add_arg:name \"%s\" value \"%s\"",args[i].name, args[i].value);
    }

    return 0;
}

int bus_invoke(bus_t *bus,char *key, char *method,int argc, bus_method_args_t *args)
{
    bus_reqhdr_t hdr;
    blob_t *blob = bus->blob;
#define BUS_ADD_OBJECT_MAX_BUFFER_LEN 1024
    uint8_t buffer[BUS_ADD_OBJECT_MAX_BUFFER_LEN];
#undef BUS_ADD_OBJECT_MAX_BUFFER_LEN 
    uint32_t buffer_len;

    dbg_str(BUS_SUC,"bus_invoke");
    /*compose req proto*/
    memset(&hdr,0,sizeof(hdr));

    hdr.type = BUS_REQ_INVOKE;

    blob_add_table_start(blob,(char *)"invoke"); {
        blob_add_string(blob, (char *)"invoke_key", key);
        blob_add_string(blob, (char *)"invoke_method", method);
        blob_add_u8(blob, (char *)"invoke_argc", argc);
        blob_add_table_start(blob, (char *)"invoke_args"); {
            bus_blob_add_args(blob,argc,args);
        }
        blob_add_table_end(blob);
    }
    blob_add_table_end(blob);

    memcpy(buffer,&hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);
    memcpy(buffer + buffer_len,(uint8_t *)blob->head,blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    /*send req proto*/
    dbg_buf(BUS_DETAIL,"bus send:",buffer,buffer_len);
    bus_send(bus, buffer, buffer_len);

    return 0;
}

int bus_invoke_async(bus_t *bus,char *key, char *method,int argc, char **args)
{
/*
 *    bus_req_t req;
 *
 *    bus_invoke(bus,key, method,argc, args);
 *
 *    req.method = method;
 *    req.state = -1;
 *
 *    make_pair(bus->pair,method,&req);
 *    hash_map_insert_data(bus->obj_hmap,bus->pair->data);
 */

    return 0;
}

int bus_invoke_sync(bus_t *bus,char *key, char *method,int argc, bus_method_args_t *args,char *out_buf,char *out_len)
{
    bus_req_t *req;
    Map *map = bus->req_map;
    int ret;
    int count = 0;
    int state = 0;
#define MAX_BUFFER_LEN 2048
    char buffer[MAX_BUFFER_LEN];
#undef MAX_BUFFER_LEN

    req = (bus_req_t *)allocator_mem_alloc(bus->allocator, sizeof(bus_req_t));
    req->method            = method;
    req->state             = 0xffff;
    req->opaque_len        = 0;
    req->opaque            = buffer;
    req->opaque_buffer_len = sizeof(buffer);

    map->add(map, method, req);

    bus_invoke(bus,key, method,argc, args);

    while (req->state == 0xffff) {
        sleep(1);
        count++;
        if (count > 5) {
            req->state = -99;
            break;
        }
    }

    dbg_str(BUS_SUC,"bus_invoke_sync,rev return state =%d",req->state);
    dbg_buf(BUS_SUC,"opaque:",req->opaque,req->opaque_len);

    memcpy(out_buf,req->opaque,req->opaque_len);
    *out_len = req->opaque_len;

    //... remove req
    
    return req->state;
}

int bus_handle_invoke_reply(bus_t *bus,  blob_attr_t **attr)
{
    char *obj_name, *method_name;
    Map *map = bus->req_map;
    bus_req_t *req;
    int state;
    int ret;
    char *buffer = NULL;
    int buffer_len = 0;

    dbg_str(BUS_SUC,"bus_handle_invoke_reply");

    if (attr[BUS_STATE]) {
        state = blob_get_u32(attr[BUS_STATE]);
        dbg_str(BUS_DETAIL,"state:%d",state);
    }
    if (attr[BUS_OBJNAME]) {
        obj_name = blob_get_string(attr[BUS_OBJNAME]);
        dbg_str(BUS_DETAIL,"object name:%s",obj_name);
    }
    if (attr[BUS_INVOKE_METHOD]) {
        method_name = blob_get_string(attr[BUS_INVOKE_METHOD]);
        dbg_str(BUS_DETAIL,"method name:%s",method_name);
    }
    if (attr[BUS_OPAQUE]) {
        buffer_len = blob_get_buffer(attr[BUS_OPAQUE],(uint8_t**)&buffer);
        dbg_buf(BUS_DETAIL,"bus_handle_invoke_reply,buffer:",buffer,buffer_len);
    }

    if (method_name != NULL) {
        ret = map->search(map, method_name, (void **)&req);
        if (ret > 0) {
            req->state = state;
            if (req->opaque_buffer_len < buffer_len) {
                dbg_str(BUS_WARNNING,"opaque buffer is too small , please check");
            }
            req->opaque_len = buffer_len;
            memcpy(req->opaque,buffer,buffer_len);
            dbg_str(BUS_DETAIL,"method_name:%s,state:%d",req->method,req->state);
        }
    }

    return 0;
}

bus_handler_t 
bus_get_method_handler(bus_object_t *obj,char *method)
{
    int i;

    for (i = 0; i < obj->n_methods; i++) {
        if (!strncmp(obj->methods[i].name,method,sizeof(obj->methods[i].name))) {
            return obj->methods[i].handler;
        }
    }

    return NULL;
}

blob_policy_t * 
bus_get_policy(bus_object_t *obj,char *method)
{
    int i;

    for (i = 0; i < obj->n_methods; i++) {
        if (!strncmp(obj->methods[i].name,method,sizeof(obj->methods[i].name))) {
            return obj->methods[i].policy;
        }
    }

    return NULL;
}

int
bus_get_n_policy(bus_object_t *obj,char *method)
{
    int i;

    for (i = 0; i < obj->n_methods; i++) {
        if (!strncmp(obj->methods[i].name,method,sizeof(obj->methods[i].name))) {
            return obj->methods[i].n_policy;
        }
    }

    return -1;
}

int bus_reply_forward_invoke(bus_t *bus, char *obj_name,char *method_name, int ret, char *buf, int buf_len,int src_fd)
{
#define BUS_ADD_OBJECT_MAX_BUFFER_LEN 2048
    bus_reqhdr_t hdr;
    blob_t *blob;
    uint8_t buffer[BUS_ADD_OBJECT_MAX_BUFFER_LEN];
    uint32_t buffer_len,tmp_len;
    allocator_t *allocator = bus->allocator;

    dbg_str(BUS_SUC,"bus_reply_forward_invoke");
    memset(&hdr,0,sizeof(hdr));

    hdr.type = BUS_REPLY_FORWARD_INVOKE;

    blob = blob_create(allocator);
    if (blob == NULL) {
        dbg_str(BUS_WARNNING,"blob_create");
        return -1;
    }
    blob_init(blob);
    blob_add_table_start(blob,(char *)"reply_forward_invoke"); {
        blob_add_string(blob, (char *)"object_name", obj_name);
        blob_add_string(blob, (char *)"invoke_method", method_name);
        blob_add_u32(blob, (char *)"state", ret);
        blob_add_buffer(blob, (char *)"opaque", buf, buf_len);
        blob_add_u32(blob, (char *)"source_fd", src_fd);
    }
    blob_add_table_end(blob);

    memcpy(buffer,&hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);

    tmp_len = buffer_len + blob_get_len((blob_attr_t *)blob->head);

    if (tmp_len > BUS_ADD_OBJECT_MAX_BUFFER_LEN) {
        dbg_str(BUS_WARNNING,"buffer is too small,please check");
        return -1;
    }

    memcpy(buffer + buffer_len,(uint8_t *)blob->head,blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    dbg_buf(BUS_DETAIL,"bus send:",buffer,buffer_len);

    bus_send(bus, buffer, buffer_len);

    return 0;
#undef BUS_ADD_OBJECT_MAX_BUFFER_LEN 
}

int bus_handle_forward_invoke(bus_t *bus,  blob_attr_t **attr)
{
    bus_object_t *obj = NULL;
    Map *map = bus->obj_map;
    blob_attr_t *args = NULL;
    int argc = 0;
    char *method_name = NULL;
    char *obj_name;
    int src_fd = -1;
    hash_map_pos_t pos;
    bus_handler_t method;
    uint8_t *p;
    blob_policy_t *policy;
    int n_policy;
    struct blob_attr_s *tb[10];
#define MAX_BUFFER_LEN 2048
    char buffer[MAX_BUFFER_LEN];
    int ret, buffer_len = 9;

    dbg_str(BUS_SUC,"bus_handle_forward_invoke");

    if (attr[BUS_INVOKE_SRC_FD]) {
        src_fd = blob_get_u32(attr[BUS_INVOKE_SRC_FD]);
        dbg_str(BUS_DETAIL,"invoke src fd:%d",src_fd);
    }
    if (attr[BUS_INVOKE_METHOD]) {
        method_name = blob_get_string(attr[BUS_INVOKE_METHOD]);
        dbg_str(BUS_DETAIL,"invoke method_name:%s",method_name);
    }
    if (attr[BUS_INVOKE_ARGC]) {
        argc = blob_get_u8(attr[BUS_INVOKE_ARGC]);
        dbg_str(BUS_DETAIL,"invoke argc=%d",argc);
    }
    if (attr[BUS_INVOKE_ARGS]) {
        dbg_str(BUS_DETAIL,"invoke args");
        args = attr[BUS_INVOKE_ARGS];
    }
    if (attr[BUS_OBJNAME]) {
        obj_name = blob_get_string(attr[BUS_OBJNAME]);
        dbg_str(BUS_DETAIL,"object name:%s",obj_name);
    }

    if (method_name != NULL) {
        ret = map->search(map, obj_name, (void **)&obj);
        if (ret > 0) {
            method = bus_get_method_handler(obj,method_name);
            policy = bus_get_policy(obj,method_name);
            n_policy = bus_get_n_policy(obj,method_name);

            dbg_str(BUS_DETAIL,"policy addr:%p,size=%d",policy,ARRAY_SIZE(policy));
            blob_parse_to_attr(policy, n_policy, tb, blob_get_data(args), blob_get_data_len(args));
            ret = method(bus,argc,tb,buffer,&buffer_len);
            if (buffer_len > MAX_BUFFER_LEN) {
                dbg_str(BUS_WARNNING,"buffer is too small,please check");
            } 
            bus_reply_forward_invoke(bus,obj_name,method_name, ret, buffer, buffer_len,src_fd);
        }
    }

    return 0;
#undef MAX_BUFFER_LEN 
}

static bus_cmd_callback handlers[__BUS_REQ_LAST] = {
    [BUSD_REPLY_ADD_OBJECT] = bus_handle_add_object_reply,
    [BUSD_REPLY_LOOKUP]     = bus_handle_lookup_object_reply,
    [BUSD_REPLY_INVOKE]     = bus_handle_invoke_reply,
    [BUSD_FORWARD_INVOKE]   = bus_handle_forward_invoke,
};

static int bus_process_receiving_data_callback(void *task)
{
    bus_reqhdr_t *hdr;
    blob_attr_t *blob_attr;
    blob_attr_t *tb[__BUS_MAX];
    bus_cmd_callback cb = NULL;
    net_task_t *t = (net_task_t *)task;
    bus_t *bus = (bus_t *)t->opaque;
    int len;

    dbg_str(BUS_DETAIL,"bus_process_receiving_data_callback");
    dbg_buf(BUS_DETAIL,"task buffer:",t->buf,t->buf_len);

    hdr = (bus_reqhdr_t *)t->buf;
    blob_attr = (blob_attr_t *)(t->buf + sizeof(bus_reqhdr_t));

    if (hdr->type > __BUS_REQ_LAST) {
        dbg_str(BUS_WARNNING,"bus receive err proto type");
        return -1;
    } 

    cb = handlers[hdr->type];

    len = blob_get_data_len(blob_attr);
    blob_attr =(blob_attr_t*) blob_get_data(blob_attr);

    blob_parse_to_attr(bus_policy,
                       ARRAY_SIZE(bus_policy),
                       tb,
                       blob_attr,
                       len);

    cb(bus,tb);

    dbg_str(BUS_DETAIL,"process_rcv end");
    return 0;
}

bus_t * bus_create(allocator_t *allocator,
                   char *server_host,
                   char *server_srv,
                   char *socket_type)
{
    bus_t *bus;
    
    dbg_str(BUS_DETAIL,"bus_create");
    bus = bus_alloc(allocator);

    bus_set(bus,"client_sk_type", socket_type, 0);

    bus_init(bus,//bus_t *bus,
             server_host,//char *server_host,
             server_srv,//char *server_srv,
             bus_process_receiving_data_callback);

    return bus;
}
