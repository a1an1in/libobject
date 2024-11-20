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
#include <libobject/core/utils/blob/blob.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/Hash_Map.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/io/Ring_Buffer.h>
#include <libobject/net/bus/bus.h>

static const blob_policy_t bus_attribs[] = {
    [BUS_OBJID]         = { .name = "object_id",      .type = BLOB_TYPE_STRING }, 
    [BUS_OBJINFOS]      = { .name = "object_infos",   .type = BLOB_TYPE_STRING }, 
    [BUS_STATE]         = { .name = "state",          .type = BLOB_TYPE_UINT32 }, 
    [BUS_OPAQUE]        = { .name = "opaque",         .type = BLOB_TYPE_BUFFER }, 
    [BUS_INVOKE_SRC_FD] = { .name = "source_fd",      .type = BLOB_TYPE_UINT32 }, 
    [BUS_INVOKE_DST_FD] = { .name = "destination_fd", .type = BLOB_TYPE_UINT32 }, 
    [BUS_INVOKE_METHOD] = { .name = "invoke_method",  .type = BLOB_TYPE_STRING }, 
    [BUS_INVOKE_ARGC]   = { .name = "invoke_argc",    .type = BLOB_TYPE_UINT32 }, 
    [BUS_INVOKE_ARGS]   = { .name = "invoke_args",    .type = BLOB_TYPE_TABLE }, 
    [BUS_TIME]          = { .name = "time",           .type = BLOB_TYPE_UINT64 }, 
};

bus_t * bus_alloc(allocator_t *allocator)
{
    bus_t *b;

    b = (bus_t *)allocator_mem_alloc(allocator, sizeof(bus_t));
    if ( b == NULL) {
        dbg_str(BUS_DETAIL, "allocator_mem_alloc");
        return NULL;
    }

    memset(b, 0, sizeof(bus_t));

    b->allocator = allocator;

    return b;
}

int bus_set(bus_t *bus, char *attrib_name, char *value, int value_len)
{
    if (!strcmp(attrib_name, "client_sk_type")) {
        bus->client_sk_type = value;
    }else{
        dbg_str(BUS_WARN, "not support attrib setting, please check");
    }

    return 0;
}

int bus_init(bus_t *bus, 
             char *server_host, 
             char *server_srv, 
             int (*process_client_task_cb)(void *task))
{
    Map *map;
    if (bus->client_sk_type == NULL) {
        bus->client_sk_type = (char *)((CLIENT_TYPE_INET_TCP));
    }

    bus->server_host = server_host;
    bus->server_srv  = server_srv;

    bus->client = client(bus->allocator, 
                         bus->client_sk_type, 
                         bus->server_host, 
                         bus->server_srv);

    client_connect(bus->client, bus->server_host, bus->server_srv);
    client_trustee(bus->client, NULL, process_client_task_cb, bus);

    bus->blob = blob_create(bus->allocator);
    if (bus->blob == NULL) {
        client_destroy(bus->client);
        dbg_str(BUS_WARN, "blob_create");
        return -1;
    }
    blob_init(bus->blob);

    /*create object hash map*/
    bus->obj_map = object_new(bus->allocator, "RBTree_Map", NULL);
    map = bus->obj_map;
    map->set_cmp_func(map, string_key_cmp_func);
    bus->req_map = object_new(bus->allocator, "RBTree_Map", NULL);
    map = bus->req_map;
    map->set_cmp_func(map, string_key_cmp_func);

    return 1;
}

int bus_send(bus_t *bus, 
             void *buf, 
             size_t buf_len)
{
    int ret = 0;

    ret = client_send(bus->client, buf, buf_len, 0);
    if (ret < 0) {
        dbg_str(BUS_WARN, "bus send err");
    }

    return ret;
}

int bus_save_obj(bus_t *bus, bus_object_t *obj)
{
    Map *map = bus->obj_map;
    int ret;

    TRY {
        ret = map->contain_key(map, obj->id);
        THROW_IF(ret == 1, 0);  // 如果已经存在就不添加，恢复bus obj的时候有可能出现。
        map->add(map, obj->id, obj);
    } CATCH (ret) {}

    return ret;
}

int bus_convert_object_to_json(bus_t *bus, bus_object_t *obj, char *out)
{
    cjson_t *root, *methods, *method, *arg;
    allocator_t *allocator = bus->allocator; 
    int n_methods = obj->n_methods, n_args, i, j;
    char *tmp;

    root = cjson_create_object();
    cjson_add_item_to_object(root, "object_id", cjson_create_string(obj->id));

    if (obj->cname != NULL)
        cjson_add_item_to_object(root, "object_cname", cjson_create_string(obj->cname));

    methods = cjson_create_object();
    cjson_add_item_to_object(root, "methods", methods);

    for (i = 0; i < n_methods; i++) {
        method = cjson_create_object();
        cjson_add_item_to_object(methods, obj->methods[i].name, method);
        for (j = 0; j < obj->methods[i].n_policy; j++) {
            arg = cjson_create_object();
            char t[50];
            sprintf(t, "arg%d", j);
            cjson_add_item_to_object(method, t, arg);
            if (obj->methods[i].policy[j].type == BLOB_TYPE_UINT32) {
                cjson_add_string_to_object(arg, "name", obj->methods[i].policy[j].name);
                cjson_add_string_to_object(arg, "type", "BLOB_TYPE_UINT32");
            } else if (obj->methods[i].policy[j].type == BLOB_TYPE_UINT64) {
                cjson_add_string_to_object(arg, "name", obj->methods[i].policy[j].name);
                cjson_add_string_to_object(arg, "type", "BLOB_TYPE_UINT64");
            } else if (obj->methods[i].policy[j].type == BLOB_TYPE_STRING) {
                cjson_add_string_to_object(arg, "name", obj->methods[i].policy[j].name);
                cjson_add_string_to_object(arg, "type", "BLOB_TYPE_STRING");
            } else if (obj->methods[i].policy[j].type == BLOB_TYPE_BUFFER) {
                cjson_add_string_to_object(arg, "name", obj->methods[i].policy[j].name);
                cjson_add_string_to_object(arg, "type", "BLOB_TYPE_BUFFER");
            } else {
            }
        }
    }

    tmp = cjson_print(root);
    strncpy(out, tmp, strlen(tmp));
    cjson_delete(root);
    free(tmp);
}

int bus_add_object(bus_t *bus, bus_object_t *obj)
{
    bus_reqhdr_t hdr;
    blob_t *blob = bus->blob;
    uint8_t buffer[BLOB_BUFFER_MAX_SIZE];
    char object_infos[BLOB_BUFFER_MAX_SIZE] = {0};
    uint32_t buffer_len;

    dbg_str(BUS_VIP, "bus_add_object, object_id:%s, object cname:%s", obj->id, obj->cname);
    memset(&hdr, 0, sizeof(hdr));
    memcpy(bus->bus_object_id, obj->id, BUS_OBJECT_ID_LEN);

    hdr.type = BUS_REQ_ADD_OBJECT;
    bus_convert_object_to_json(bus, obj, object_infos);
    dbg_str(BUS_DETAIL, "object_infos:%s", object_infos);
    
    blob_reset(blob);
    blob_add_table_start(blob, (char *)"object"); {
        blob_add_string(blob, (char *)"object_id", obj->id);
        blob_add_string(blob, (char *)"object_infos", object_infos);
    }
    blob_add_table_end(blob);

    memcpy(buffer, &hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);

    memcpy(buffer + buffer_len, (uint8_t *)blob->head, 
           blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    dbg_buf(BUS_DETAIL, "bus send:", buffer, buffer_len);
    bus_send(bus, buffer, buffer_len);

    bus_save_obj(bus, obj);
    obj->bus = bus;

    return 0;
}

int bus_handle_add_object_reply(bus_t *bus, blob_attr_t **attr)
{
    int state;

    if (attr[BUS_STATE]) {
        state = blob_get_uint32(attr[BUS_STATE]);
    }

    if ( state == 1) {
        bus->bus_object_added_flag = 1;
        dbg_str(BUS_SUC, "bus_handle_add_object_reply, add obj success, object_id:%s", blob_get_string(attr[BUS_OBJID]));
    } else {
        dbg_str(BUS_ERROR, "bus_handle_add_object_reply, bus add obj failed");
        //..... del the obj
    }

    return 0;
}

int bus_lookup(bus_t *bus, char *key)
{
    bus_reqhdr_t hdr;
    blob_t *blob = bus->blob;
    uint8_t buffer[BLOB_BUFFER_MAX_SIZE];
    uint32_t buffer_len;

    memset(&hdr, 0, sizeof(hdr));

    hdr.type = BUS_REQ_LOOKUP;

    blob_add_table_start(blob, (char *)"lookup"); {
        blob_add_string(blob, (char *)"object_id", key);
    }
    blob_add_table_end(blob);

    memcpy(buffer, &hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);
    /*
     *dbg_buf(BUS_DETAIL, "object:", blob->head, blob_get_len(blob->head));
     */
    memcpy(buffer + buffer_len, (uint8_t *)blob->head,
           blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    dbg_buf(BUS_DETAIL, "bus send:", buffer, buffer_len);
    bus_send(bus, buffer, buffer_len);

    return 0;
}

int bus_lookup_sync(bus_t *bus, char *object_id, char *buffer, int *buffer_len)
{
    bus_req_t *req = NULL;
    Map *map = bus->req_map;
    int ret;

    TRY {
        THROW_IF(buffer_len == NULL || buffer == NULL, -1);

        req = (bus_req_t *)allocator_mem_zalloc(bus->allocator, sizeof(bus_req_t));
        req->state             = 0xfffe;
        req->opaque_len        = 0;
        req->opaque            = (uint8_t *)buffer;
        req->opaque_buffer_len = *buffer_len;

        sprintf(buffer, "%s@lookup", object_id);
        EXEC(map->add(map, buffer, req));
        EXEC(bus_lookup(bus, object_id));

        while(req->state == 0xfffe) usleep(100);

        dbg_str(BUS_VIP, "lookup result:%s", req->opaque);
        *buffer_len = req->opaque_len;
        EXEC(map->del(map, buffer));
    } CATCH (ret) {} FINALLY {
        allocator_mem_free(bus->allocator, req);
    }
    
    return ret;
}

int bus_handle_lookup_object_reply(bus_t *bus, blob_attr_t **attr)
{
    bus_object_t *obj;
    blob_attr_t *attrib, *head;
    Map *map = bus->req_map;
    char *object_id, *infos = NULL;
    bus_req_t *req;
    uint32_t len;
#define MAX_BUFFER_LEN 2048
    char key[MAX_BUFFER_LEN] = {0};
#undef MAX_BUFFER_LEN
    int ret;

    TRY {
        dbg_str(BUS_DETAIL, "bus_handle_lookup_object_reply");

        if (attr[BUS_OBJID]) {
            object_id = blob_get_string(attr[BUS_OBJID]);
            dbg_str(BUS_DETAIL, "object_id:%s", object_id);
        }
        if (attr[BUS_OBJINFOS]) {
            infos = blob_get_string(attr[BUS_OBJINFOS]);
        }

        sprintf(key, "%s@lookup", object_id);
        ret = map->search(map, key, (void **)&req);
        THROW_IF(ret <= 0, -1);
        THROW_IF(strlen(infos) > req->opaque_buffer_len, -1);

        if (infos != NULL) {
            memset(req->opaque, 0, req->opaque_buffer_len);
            memcpy(req->opaque, infos, strlen(infos));
            req->opaque_len = strlen(infos);
            req->state = 1;
        }
  
    } CATCH (ret) {}

    return ret;
}

int bus_blob_add_args(blob_t *blob, int argc, bus_method_args_t *args)
{
    int i, ret;

    TRY {
        for (i = 0; i < argc; i++) {
            if (args[i].type == ARG_TYPE_STRING) {
                blob_add_string(blob, (char *)args[i].name, args[i].value);
            } else if (args[i].type == ARG_TYPE_UINT32) {
                blob_add_uint32(blob, (char *)args[i].name, (uint32_t)args[i].value);
            } else if (args[i].type == ARG_TYPE_UINT64) {
                blob_add_uint64(blob, (char *)args[i].name, (uint64_t)args[i].value);
            } else if (args[i].type == ARG_TYPE_INT32) {
                blob_add_int32(blob, (char *)args[i].name, (int32_t)args[i].value);
            } else if (args[i].type == ARG_TYPE_BUFFER) {
                THROW_IF(args[i].len > BLOB_BUFFER_MAX_SIZE, -1);
                blob_add_buffer(blob, (char *)args[i].name, (uint8_t *)args[i].value, args[i].len);
            } else {
                dbg_str(BUS_WARN, "bus_blob_add_args, not support type = %d", args[i].type);
            }
            // dbg_str(BUS_DETAIL, "bus_blob_add_arg:name \"%s\" value \"%s\"",
            //         args[i].name, args[i].value);
        }
    } CATCH (ret) {}

    return ret;
}

int 
bus_invoke(bus_t *bus, char *object_id, char *method, 
           int argc, bus_method_args_t *args)
{
    bus_reqhdr_t hdr;
    blob_t *blob = bus->blob;
    uint8_t buffer[BLOB_MAX_SIZE];
    uint32_t buffer_len;
    int ret;

    TRY {
        dbg_str(BUS_INFO, "bus_invoke, object_id:%s, method:%s", object_id, method);
        /*compose req proto*/
        memset(&hdr, 0, sizeof(hdr));

        hdr.type = BUS_REQ_INVOKE;
        blob_reset(blob);

        blob_add_table_start(blob, (char *)"invoke"); {
            blob_add_string(blob, (char *)"object_id", object_id);
            blob_add_string(blob, (char *)"invoke_method", method);
            blob_add_uint8(blob, (char *)"invoke_argc", argc);
            blob_add_table_start(blob, (char *)"invoke_args"); {
                EXEC(bus_blob_add_args(blob, argc, args));
            }
            blob_add_table_end(blob);
        }
        blob_add_table_end(blob);

        memcpy(buffer, &hdr, sizeof(hdr));
        buffer_len = sizeof(hdr);
        memcpy(buffer + buffer_len, (uint8_t *)blob->head, 
            blob_get_len((blob_attr_t *)blob->head));
        buffer_len += blob_get_len((blob_attr_t *)blob->head);

        /*send req proto*/
        dbg_buf(BUS_DETAIL, "bus send:", buffer, buffer_len);
        EXEC(bus_send(bus, buffer, buffer_len));
    } CATCH (ret) {}

    return ret;
}

int bus_invoke_async(bus_t *bus, char *object_id, char *method,
                     int argc, bus_method_args_t *args, 
                     void *async_callback, void *opaque)
{
    bus_req_t *req = NULL;
    Map *map = bus->req_map;
    int count = 0, state = 0;
    int ret;

    TRY {
        req = (bus_req_t *)allocator_mem_zalloc(bus->allocator, sizeof(bus_req_t));
        req->method            = method;
        req->async_callback    = async_callback;
        req->opaque            = opaque;

        sprintf(req->key, "%s@%s", object_id, method);
        EXEC(map->add(map, req->key, req));
        dbg_str(BUS_VIP, "bus_invoke_async req key:%s", req->key);

        EXEC(bus_invoke(bus, object_id, method, argc, args));
    } CATCH (ret) {} FINALLY {}
 
    return ret;
}

int bus_invoke_sync(bus_t *bus, char *object_id, char *method,
                    int argc, bus_method_args_t *args, 
                    char *out_buf, uint32_t *out_len)
{
    bus_req_t *req = NULL;
    Map *map = bus->req_map;
    int count = 0, state = 0;
    int ret;

    TRY {
        req = (bus_req_t *)allocator_mem_zalloc(bus->allocator, sizeof(bus_req_t));
        req->method            = method;
        req->state             = 0xfffe;
        req->opaque_len        = 0;
        req->opaque            = (uint8_t *)out_buf;
        req->opaque_buffer_len = (out_len == NULL) ? 0 : *out_len;
        dbg_str(BUS_INFO, "bus_invoke_sync, opaque_buffer_len=%d, out_len addr:%p", req->opaque_buffer_len, out_len);

        sprintf(req->key, "%s@%s", object_id, method);
        EXEC(map->add(map, req->key, req));
        dbg_str(BUS_INFO, "bus_invoke_sync, req count=%d", map->count(map));

        EXEC(bus_invoke(bus, object_id, method, argc, args));

        while(req->state == 0xfffe) usleep(100);

        dbg_str(BUS_VIP, "bus_invoke_sync method:%s, return state=%d, opaque:%p, opaque_len:%d", method, req->state, req->opaque, req->opaque_len);

        if (out_buf != NULL) {
            dbg_buf(BUS_DETAIL, "opaque:", req->opaque, req->opaque_len);
            *out_len = req->opaque_len;
        }

        EXEC(map->del(map, req->key));
        THROW(req->state);
    } CATCH (ret) {} FINALLY {
        allocator_mem_free(bus->allocator, req);
    }
 
    return ret;
}

int bus_handle_invoke_reply(bus_t *bus, blob_attr_t **attr)
{
    char *object_id, *method_name;
    Map *map = bus->req_map;
    bus_req_t *req;
    int state;
    int ret;
    char *buffer = NULL;
    int buffer_len = 0;
    int (*async_callback)(bus_req_t *req, char *out, int len, int state, void *opaque);
#define MAX_BUFFER_LEN 2048
    char key[MAX_BUFFER_LEN] = {0};
#undef MAX_BUFFER_LEN

    if (attr[BUS_STATE]) {
        state = blob_get_uint32(attr[BUS_STATE]);
    }
    if (attr[BUS_OBJID]) {
        object_id = blob_get_string(attr[BUS_OBJID]);
    }
    if (attr[BUS_INVOKE_METHOD]) {
        method_name = blob_get_string(attr[BUS_INVOKE_METHOD]);
    }
    if (attr[BUS_OPAQUE]) {
        buffer_len = blob_get_buffer(attr[BUS_OPAQUE], (uint8_t**)&buffer);
    }

    dbg_str(BUS_VIP, "bus_handle_invoke_reply, object_id:%s, method:%s, state:%d, buffer len:%d", object_id, method_name, state, buffer_len);

    if (method_name != NULL) {
        sprintf(key, "%s@%s", object_id, method_name);
        ret = map->search(map, key, (void **)&req);
        if (ret <= 0) return ret;
        if (req->async_callback == NULL) {
            if (req->opaque_buffer_len < buffer_len) {
                req->opaque_len = req->opaque_buffer_len;
                dbg_str(BUS_WARN, "opaque buffer is too small, please check, opaque_buffer_len:%d, buffer_len:%d", req->opaque_buffer_len, buffer_len);
            } else {
                req->opaque_len = buffer_len;
            }
            
            if (req->opaque) {
                memcpy(req->opaque, buffer, req->opaque_len);
                 dbg_buf(BUS_DETAIL, "bus buffer:", buffer, req->opaque_len);
            }
            req->state = state;
        } else {
            dbg_buf(BUS_VIP, "bus async buffer:", buffer, buffer_len);
            async_callback = req->async_callback;
            async_callback(req, buffer, buffer_len, state, req->opaque);
        }
    }

    return 0;
}

bus_handler_t 
bus_get_method_handler(bus_object_t *obj, char *method)
{
    int i;

    for (i = 0; i < obj->n_methods; i++) {
        if (!strncmp(obj->methods[i].name, method, strlen(method)) && 
            (strlen(method) == strlen(obj->methods[i].name))) {
            return obj->methods[i].handler;
        }
    }

    return NULL;
}

blob_policy_t * 
bus_get_policy(bus_object_t *obj, char *method)
{
    int i;

    for (i = 0; i < obj->n_methods; i++) {
        if (!strncmp(obj->methods[i].name, method, strlen(method)) && 
            (strlen(method) == strlen(obj->methods[i].name))) {
            return obj->methods[i].policy;
        }
    }

    return NULL;
}

int
bus_get_n_policy(bus_object_t *obj, char *method)
{
    int i;

    for (i = 0; i < obj->n_methods; i++) {
        if (!strncmp(obj->methods[i].name, method, strlen(method)) && 
            (strlen(method) == strlen(obj->methods[i].name))) {
            return obj->methods[i].n_policy;
        }
    }

    return -1;
}

int 
bus_reply_forward_invoke(bus_t *bus, char *object_id,
                         char *method_name, int state, char *buf,
                         int buf_len, int src_fd)
{
    bus_reqhdr_t hdr;
    blob_t *blob = bus->blob;
    uint8_t buffer[BLOB_BUFFER_MAX_SIZE];
    uint32_t buffer_len, tmp_len;
    allocator_t *allocator = bus->allocator;

    dbg_str(BUS_VIP, "bus_reply_forward_invoke, object_id:%s, method_name:%s, state:%d, src_fd:%d", 
            object_id, method_name, state, src_fd);
    memset(&hdr, 0, sizeof(hdr));

    hdr.type = BUS_REPLY_FORWARD_INVOKE;

    blob_reset(blob);
    blob_add_table_start(blob, (char *)"reply_forward_invoke"); {
        blob_add_string(blob, (char *)"object_id", object_id);
        blob_add_string(blob, (char *)"invoke_method", method_name);
        blob_add_uint32(blob, (char *)"state", state);
        blob_add_buffer(blob, (char *)"opaque", (uint8_t *)buf, buf_len);
        blob_add_uint32(blob, (char *)"source_fd", src_fd);
    }
    blob_add_table_end(blob);

    memcpy(buffer, &hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);

    tmp_len = buffer_len + blob_get_len((blob_attr_t *)blob->head);

    if (tmp_len > BLOB_BUFFER_MAX_SIZE) {
        dbg_str(BUS_WARN, "buffer is too small, please check");
        return -1;
    }

    memcpy(buffer + buffer_len, (uint8_t *)blob->head,
           blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    dbg_buf(BUS_VIP, "bus send:", buffer, buffer_len);

    bus_send(bus, buffer, buffer_len);

    return 0;
}

int bus_handle_forward_invoke(bus_t *bus, blob_attr_t **attr)
{
    bus_object_t *obj = NULL;
    Map *map = bus->obj_map;
    blob_attr_t *args = NULL;
    int argc = 0;
    char *method_name = NULL;
    char *object_id;
    int src_fd = -1;
    hash_map_pos_t pos;
    bus_handler_t method;
    uint8_t *p;
    blob_policy_t *policy;
    int n_policy;
    struct blob_attr_s *tb[10];
    char buffer[BLOB_BUFFER_MAX_SIZE];
    int ret, buffer_len = 9;

    if (attr[BUS_INVOKE_SRC_FD]) {
        src_fd = blob_get_uint32(attr[BUS_INVOKE_SRC_FD]);
    }
    if (attr[BUS_INVOKE_METHOD]) {
        method_name = blob_get_string(attr[BUS_INVOKE_METHOD]);
    }
    if (attr[BUS_INVOKE_ARGC]) {
        argc = blob_get_uint8(attr[BUS_INVOKE_ARGC]);
    }
    if (attr[BUS_INVOKE_ARGS]) {
        args = attr[BUS_INVOKE_ARGS];
    }
    if (attr[BUS_OBJID]) {
        object_id = blob_get_string(attr[BUS_OBJID]);
    }

    if (method_name != NULL) {
        ret = map->search(map, object_id, (void **)&obj);
        if (ret > 0) {
            method = bus_get_method_handler(obj, method_name);
            policy = bus_get_policy(obj, method_name);
            n_policy = bus_get_n_policy(obj, method_name);
            obj->src_fd = src_fd;

            blob_parse_to_attr(policy, n_policy, tb, blob_get_data(args),
                               blob_get_data_len(args));
            ret = method(obj, argc, tb, buffer, &buffer_len);
            if (buffer_len > BLOB_BUFFER_MAX_SIZE) {
                dbg_str(BUS_WARN, "buffer is too small, please check");
            } 
            bus_reply_forward_invoke(bus, object_id, method_name, ret,
                                     buffer, buffer_len, src_fd);
        } else {
            dbg_str(BUS_DETAIL, "bus_handle_forward_invoke can't find object_id:%s", object_id);
        }
    }

    return 0;
}

int bus_ping(bus_t *bus)
{
    bus_reqhdr_t hdr;
    blob_t *blob = bus->blob;
    uint8_t buffer[BLOB_BUFFER_MAX_SIZE];
    uint32_t buffer_len;

    memset(&hdr, 0, sizeof(hdr));
    hdr.type = BUS_REQ_PING;

    memcpy(buffer, &hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);

    blob_reset(blob);
    blob_add_table_start(blob, (char *)"TTL"); {
        blob_add_uint32(blob, (char *)"time", 9);
    }
    blob_add_table_end(blob);

    memcpy(buffer + buffer_len, (uint8_t *)blob->head, 
           blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    dbg_buf(BUS_DETAIL, "bus send ping:", buffer, buffer_len);
    bus_send(bus, buffer, buffer_len);

    return 0;
}

int bus_handle_pong_reply(bus_t *bus, blob_attr_t **attr)
{
    uint32_t time;

    if (attr[BUS_TIME]) {
        time = blob_get_uint32(attr[BUS_TIME]);
    }
    bus->bus_object_off_line_flag = 0;
    dbg_str(BUS_VIP, "bus_handle_pong_reply, time:%d", 0);

    return 0;
}

static bus_cmd_callback handlers[__BUS_REQ_LAST] = {
    [BUSD_REPLY_ADD_OBJECT] = bus_handle_add_object_reply, 
    [BUSD_REPLY_LOOKUP]     = bus_handle_lookup_object_reply, 
    [BUSD_REPLY_INVOKE]     = bus_handle_invoke_reply, 
    [BUSD_FORWARD_INVOKE]   = bus_handle_forward_invoke, 
    [BUSD_REPLY_PONG]       = bus_handle_pong_reply, 
};

static int bus_process_receiving_data_callback(void *task)
{
    bus_reqhdr_t *hdr;
    blob_attr_t *blob_attr;
    blob_attr_t *tb[__BUS_MAX];
    bus_cmd_callback cb = NULL;
    work_task_t *t = (work_task_t *)task;
    bus_t *bus = (bus_t *)t->opaque;
    allocator_t *allocator = bus->allocator;
    Ring_Buffer *rb;
    char buffer[BLOB_BUFFER_MAX_SIZE];
    int len, buffer_len, blob_table_len, ret;

    TRY {
        THROW_IF(t->buf_len <= 0, 0);
        if (t->buf_len > 2 && ((char *)t->buf)[1] != 4)
            dbg_buf(BUS_VIP, "bus receive:", t->buf, t->buf_len);

        /* 1.将数据写入cache */
        if (t->cache == NULL) {
            t->cache = object_new(allocator, "Ring_Buffer", NULL);
            rb = t->cache;
            rb->set_size(rb, BLOB_BUFFER_MAX_SIZE);
            dbg_str(BUS_DETAIL, "new buffer");
        } else {
            rb = t->cache;
        }

        rb->write(rb, t->buf, t->buf_len);
        buffer_len = rb->get_len(rb);
    
        do {
            /* 2.判断数据类型 */
            THROW_IF(buffer_len < sizeof(bus_reqhdr_t) + sizeof(blob_attr_t), 0); //数据小于req和table头，返回等待下一次读信号。
            rb->peek(rb, buffer, sizeof(bus_reqhdr_t));  //数据可能没有收齐，所以需要先peek一下
            hdr = (bus_reqhdr_t *)buffer;
            THROW_IF(hdr->type > __BUS_REQ_LAST, -1);
            dbg_str(BUS_DETAIL, "type:%d", hdr->type);

            /* 3.获取blob table和table len， 如果Cache的数据不够blob table的长度，
            *   说明数据有可能有分片，需要返回直到收集完全后往后处理。 */
            rb->peek(rb, buffer, sizeof(bus_reqhdr_t) + sizeof(blob_attr_t));
            blob_attr = (blob_attr_t *)((char *)buffer + sizeof(bus_reqhdr_t));
            blob_table_len = blob_get_len(blob_attr);
            dbg_str(BUS_DETAIL, "blob_table_len:%d, buffer_len:%d", blob_table_len, buffer_len);
            THROW_IF(buffer_len - sizeof(bus_reqhdr_t) < blob_table_len, 0); //数据没有收齐，返回等待下一次读信号。
            // THROW_IF(blob_table_len < buffer_len - sizeof(bus_reqhdr_t), -1); //数据异常，接受到比期望多的数据。
            
            /* 4.获取table内容长度和内容的首地址 */
            rb->read(rb, buffer, sizeof(bus_reqhdr_t) + blob_table_len);
            blob_attr = (blob_attr_t *)((char *)buffer + sizeof(bus_reqhdr_t));
            len = blob_get_data_len(blob_attr);
            blob_attr =(blob_attr_t*) blob_get_data(blob_attr);

            dbg_str(BUS_DETAIL, "busd receive, blob expect blob_table_len:%d, buffer len:%d, head len:%d, receive len:%d", 
                    blob_table_len, buffer_len, sizeof(bus_reqhdr_t), t->buf_len);

            /* 5.解析bus attribs */
            EXEC(blob_parse_to_attr(bus_attribs, 
                                    ARRAY_SIZE(bus_attribs), 
                                    tb,
                                    blob_attr, 
                                    len));

            /* 6.调用相应回调处理数据 */
            cb = handlers[hdr->type];
            THROW_IF(cb == NULL, -1);
            EXEC(cb(bus, tb));
            buffer_len = rb->get_len(rb);
            dbg_str(BUS_DETAIL, "left buffer_len:%d", buffer_len);
        } while (buffer_len > 0);
    } CATCH (ret) {} FINALLY {
        if ((buffer_len == 0) || (t->buf_len <= 0) || ret < 0) {
            object_destroy(t->cache);
            t->cache = NULL;
            dbg_str(BUS_DETAIL, "release task cache");
        }

        /* 如果是bus service断链了，需要恢复*/
        if ((t->buf_len <= 0) && (bus->bus_object_added_flag == 1)) {
            bus->bus_object_off_line_flag = 1;
            // client_revoke(bus->client);
            client_destroy(bus->client);
            bus->client = NULL;
        }
    }
    
    return ret;
}

int bus_obj_map_add_obj_foreach_callback(void *key, void *element, void *arg)
{
    bus_t *bus = (bus_t *)arg;
    int ret;

    TRY {
        if (element != NULL) bus_add_object(bus, element);
    } CATCH (ret) {}

    return ret;
}

static void bus_timer_callback(void *task)
{
    bus_t *bus = (bus_t *)task;
    Map *map = bus->obj_map;
    int ret;
    
    TRY {
        if (bus->bus_object_off_line_flag == 1) {
            dbg_str(DBG_INFO, "bus_timer_callback have detected the object_id:%s offline", bus->bus_object_id);
            bus->client = client(bus->allocator,  bus->client_sk_type, bus->server_host, bus->server_srv);
            EXEC(client_connect(bus->client, bus->server_host, bus->server_srv));
            EXEC(client_trustee(bus->client, NULL, bus_process_receiving_data_callback, bus));

            map->for_each_arg(map, bus_obj_map_add_obj_foreach_callback, bus); // 断链后重新添加obj
            bus->bus_object_off_line_flag = 0;
        } else {
            EXEC(bus_ping(bus));
        }
        
    } CATCH (ret) {}
}

bus_t * bus_create(allocator_t *allocator, 
                   char *server_host, 
                   char *server_srv, 
                   char *socket_type)
{
    bus_t *bus = NULL;
    struct timeval *ev_tv;
    int ret;
    
    TRY {
        dbg_str(BUS_DETAIL, "bus_create");
        bus = bus_alloc(allocator);

        bus_set(bus, "client_sk_type", socket_type, 0);

        EXEC(bus_init(bus, server_host, server_srv, bus_process_receiving_data_callback));

        /* 构造一个定时器， 如果bus是service， 则会60 * 2秒尝试恢复链接， 后面会用退避算法优化这个定时器时间。 */
        ev_tv = &bus->ev_tv;
        ev_tv->tv_sec = 60;
        ev_tv->tv_usec = 0;
        bus->timer_worker = timer_worker(allocator, EV_READ | EV_PERSIST, ev_tv, bus_timer_callback, bus);
    } CATCH (ret) { bus = NULL; }

    return bus;
}

int bus_destroy(bus_t *bus)
{
    if (bus == NULL) return 0;

    object_destroy(bus->obj_map);
    object_destroy(bus->req_map);
    blob_destroy(bus->blob);
    client_destroy(bus->client);
    client_destroy(bus->timer_worker);
    allocator_mem_free(bus->allocator, bus);

    return 1;
}
