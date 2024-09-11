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

int __bus_add_obj(bus_t *bus, bus_object_t *obj)
{
    Map *map = bus->obj_map;

    return map->add(map, obj->id, obj);
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

    dbg_str(DBG_VIP, "bus_add_object, object_id:%s, object cname:%s", obj->id, obj->cname);
    memset(&hdr, 0, sizeof(hdr));

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

    __bus_add_obj(bus, obj);
    obj->bus = bus;

    return 0;
}

int bus_handle_add_object_reply(bus_t *bus, blob_attr_t **attr)
{
    int state;

    dbg_str(BUS_DETAIL, "bus_handle_add_object_reply");

    if (attr[BUS_STATE]) {
        state = blob_get_uint32(attr[BUS_STATE]);
        dbg_str(BUS_DETAIL, "state=%d", state);
    }

    if (attr[BUS_OBJID]) {
        dbg_str(BUS_DETAIL, "object_id:%s", blob_get_string(attr[BUS_OBJID]));
    }

    if ( state == 1) {
        dbg_str(BUS_SUC, "bus add obj success");
    } else {
        dbg_str(BUS_ERROR, "bus add obj failed");
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

    /*
     *dbg_str(BUS_DETAIL, "run at here");
     */
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
        dbg_str(BUS_SUC, "bus_invoke, object_id:%s, method:%s", object_id, method);
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

int bus_invoke_async(bus_t *bus, char *key, char *method, int argc, char **args)
{
/*
 *    bus_req_t req;
 *
 *    bus_invoke(bus, key, method, argc, args);
 *
 *    req.method = method;
 *    req.state = -1;
 *
 *    make_pair(bus->pair, method, &req);
 *    hash_map_insert_data(bus->obj_hmap, bus->pair->data);
 */

    return 0;
}

int
bus_invoke_sync(bus_t *bus, char *object_id, char *method,
                int argc, bus_method_args_t *args, 
                char *out_buf, uint32_t *out_len)
{
    bus_req_t *req = NULL;
    Map *map = bus->req_map;
    int count = 0, state = 0;
    char buffer[BLOB_BUFFER_MAX_SIZE] = {0};
    int ret;

    TRY {
        req = (bus_req_t *)allocator_mem_zalloc(bus->allocator, sizeof(bus_req_t));
        req->method            = method;
        req->state             = 0xfffe;
        req->opaque_len        = 0;
        req->opaque            = (uint8_t *)out_buf;
        req->opaque_buffer_len = (out_len == NULL) ? 0 : *out_len;
        dbg_str(BUS_INFO, "bus_invoke_sync, opaque_buffer_len=%d, out_len addr:%p", req->opaque_buffer_len, out_len);

        sprintf(buffer, "%s@%s", object_id, method);
        EXEC(map->add(map, buffer, req));
        dbg_str(BUS_INFO, "bus_invoke_sync, req count=%d", map->count(map));

        EXEC(bus_invoke(bus, object_id, method, argc, args));

        while(req->state == 0xfffe) usleep(100);

        dbg_str(BUS_VIP, "bus_invoke_sync method:%s, return state=%d, opaque:%p, opaque_len:%d", method, req->state, req->opaque, req->opaque_len);

        if (out_buf != NULL) {
            dbg_buf(BUS_DETAIL, "opaque:", req->opaque, req->opaque_len);
            *out_len = req->opaque_len;
        }

        EXEC(map->del(map, buffer));
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
#define MAX_BUFFER_LEN 2048
    char key[MAX_BUFFER_LEN] = {0};
#undef MAX_BUFFER_LEN

    if (attr[BUS_STATE]) {
        state = blob_get_uint32(attr[BUS_STATE]);
        dbg_str(BUS_DETAIL, "state:%d", state);
    }
    if (attr[BUS_OBJID]) {
        object_id = blob_get_string(attr[BUS_OBJID]);
        dbg_str(BUS_DETAIL, "object_id:%s", object_id);
    }
    if (attr[BUS_INVOKE_METHOD]) {
        method_name = blob_get_string(attr[BUS_INVOKE_METHOD]);
        dbg_str(BUS_DETAIL, "method name:%s", method_name);
    }
    if (attr[BUS_OPAQUE]) {
        buffer_len = blob_get_buffer(attr[BUS_OPAQUE], (uint8_t**)&buffer);
        dbg_buf(BUS_DETAIL, "bus_handle_invoke_reply, buffer:", (uint8_t *)buffer, buffer_len);
    }

    dbg_str(BUS_SUC, "bus_handle_invoke_reply, object_id:%s, method:%s", object_id, method_name);

    if (method_name != NULL) {
        sprintf(key, "%s@%s", object_id, method_name);
        ret = map->search(map, key, (void **)&req);
        if (ret > 0) {
            if (req->opaque_buffer_len < buffer_len) {
                req->opaque_len = req->opaque_buffer_len;
                dbg_str(BUS_WARN, "opaque buffer is too small, please check, opaque_buffer_len:%d, buffer_len:%d", req->opaque_buffer_len, buffer_len);
            } else {
                req->opaque_len = buffer_len;
            }
            
            if (req->opaque) {
                memcpy(req->opaque, buffer, req->opaque_len);
                 dbg_buf(BUS_VIP, "bus buffer:", buffer, req->opaque_len);
            }
            req->state = state;
            dbg_str(BUS_INFO, "method_name:%s, state:%d", req->method, req->state);
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
                         char *method_name, int ret, char *buf,
                         int buf_len, int src_fd)
{
    bus_reqhdr_t hdr;
    blob_t *blob = bus->blob;
    uint8_t buffer[BLOB_BUFFER_MAX_SIZE];
    uint32_t buffer_len, tmp_len;
    allocator_t *allocator = bus->allocator;

    dbg_str(BUS_VIP, "bus_reply_forward_invoke, buf_len:%d", buf_len);
    memset(&hdr, 0, sizeof(hdr));

    hdr.type = BUS_REPLY_FORWARD_INVOKE;

    blob_reset(blob);
    blob_add_table_start(blob, (char *)"reply_forward_invoke"); {
        blob_add_string(blob, (char *)"object_id", object_id);
        blob_add_string(blob, (char *)"invoke_method", method_name);
        blob_add_uint32(blob, (char *)"state", ret);
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

    dbg_buf(BUS_DETAIL, "bus send:", buffer, buffer_len);

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

    dbg_str(BUS_DETAIL, "bus_handle_forward_invoke");

    if (attr[BUS_INVOKE_SRC_FD]) {
        src_fd = blob_get_uint32(attr[BUS_INVOKE_SRC_FD]);
        dbg_str(BUS_DETAIL, "invoke src fd:%d", src_fd);
    }
    if (attr[BUS_INVOKE_METHOD]) {
        method_name = blob_get_string(attr[BUS_INVOKE_METHOD]);
        dbg_str(BUS_DETAIL, "invoke method_name:%s", method_name);
    }
    if (attr[BUS_INVOKE_ARGC]) {
        argc = blob_get_uint8(attr[BUS_INVOKE_ARGC]);
        dbg_str(BUS_DETAIL, "invoke argc=%d", argc);
    }
    if (attr[BUS_INVOKE_ARGS]) {
        dbg_str(BUS_DETAIL, "invoke args");
        args = attr[BUS_INVOKE_ARGS];
    }
    if (attr[BUS_OBJID]) {
        object_id = blob_get_string(attr[BUS_OBJID]);
        dbg_str(BUS_DETAIL, "object_id:%s", object_id);
    }

    if (method_name != NULL) {
        ret = map->search(map, object_id, (void **)&obj);
        if (ret > 0) {
            method = bus_get_method_handler(obj, method_name);
            policy = bus_get_policy(obj, method_name);
            n_policy = bus_get_n_policy(obj, method_name);

            dbg_str(BUS_DETAIL, "policy addr:%p, size=%d", policy, ARRAY_SIZE(policy));
            blob_parse_to_attr(policy, n_policy, tb, blob_get_data(args),
                               blob_get_data_len(args));
            ret = method(obj, argc, tb, buffer, &buffer_len);
            if (buffer_len > BLOB_BUFFER_MAX_SIZE) {
                dbg_str(BUS_WARN, "buffer is too small, please check");
            } 
            bus_reply_forward_invoke(bus, object_id, method_name, ret,
                                     buffer, buffer_len, src_fd);
        }
    }

    return 0;
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
    work_task_t *t = (work_task_t *)task;
    bus_t *bus = (bus_t *)t->opaque;
    int len;

    dbg_str(BUS_DETAIL, "bus_process_receiving_data_callback");
    dbg_buf(BUS_DETAIL, "task buffer:", t->buf, t->buf_len);

    hdr = (bus_reqhdr_t *)t->buf;
    blob_attr = (blob_attr_t *)(t->buf + sizeof(bus_reqhdr_t));

    if (hdr->type > __BUS_REQ_LAST) {
        dbg_str(BUS_WARN, "bus receive err proto type");
        return -1;
    } 

    cb = handlers[hdr->type];

    len = blob_get_data_len(blob_attr);
    blob_attr =(blob_attr_t*) blob_get_data(blob_attr);

    dbg_str(BUS_DETAIL, "bus_attribs size:%d", ARRAY_SIZE(bus_attribs));

    blob_parse_to_attr(bus_attribs, 
                       ARRAY_SIZE(bus_attribs), 
                       tb, 
                       blob_attr, 
                       len);

    cb(bus, tb);

    dbg_str(BUS_DETAIL, "process_rcv end");
    return 0;
}

bus_t * bus_create(allocator_t *allocator, 
                   char *server_host, 
                   char *server_srv, 
                   char *socket_type)
{
    bus_t *bus = NULL;
    int ret;
    
    TRY {
        dbg_str(BUS_DETAIL, "bus_create");
        bus = bus_alloc(allocator);

        bus_set(bus, "client_sk_type", socket_type, 0);

        EXEC(bus_init(bus, server_host, server_srv, bus_process_receiving_data_callback));
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
    allocator_mem_free(bus->allocator, bus);

    return 1;
}
