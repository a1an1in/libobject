/**
 * @file busd.c
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
#include <libobject/net/bus/busd.h>
#include <libobject/net/bus/bus.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/miscellany/buffer.h>
#include <libobject/core/utils/config.h>
#include <libobject/core/Hash_Map.h>
#include <libobject/core/io/Ring_Buffer.h>

static const struct blob_policy_s busd_attribs[] = {
    [BUSD_OBJID]          = { .name = "object_id",      .type = BLOB_TYPE_STRING },
    [BUSD_OBJINFOS]       = { .name = "object_infos",   .type = BLOB_TYPE_STRING }, 
    [BUSD_INVOKE_METHORD] = { .name = "invoke_method",  .type = BLOB_TYPE_STRING }, 
    [BUSD_INVOKE_ARGC]    = { .name = "invoke_argc",    .type = BLOB_TYPE_UINT32 }, 
    [BUSD_INVOKE_ARGS]    = { .name = "invoke_args",    .type = BLOB_TYPE_STRING }, 
    [BUSD_STATE]          = { .name = "state",          .type = BLOB_TYPE_UINT32 }, 
    [BUSD_OPAQUE]         = { .name = "opaque",         .type = BLOB_TYPE_UINT32 }, 
    [BUSD_INVOKE_SRC_FD]  = { .name = "source_fd",      .type = BLOB_TYPE_UINT32 },
    [BUSD_TIME]           = { .name = "time",           .type = BLOB_TYPE_UINT64 }, 
};

int busd_dump_object(struct busd_object *obj)
{
    dbg_str(BUS_DETAIL, "busd_dump_object:%s", obj->infos);

    return 0;
}

void busd_release_object(void *key, void *element)
{
    struct busd_object *obj = (struct busd_object *)element;
    allocator_t *allocator = obj->allocator;

    allocator_mem_free(allocator, obj->infos);
    allocator_mem_free(allocator, obj);
}


busd_t * busd_alloc(allocator_t *allocator)
{
    busd_t *d;
    blob_t *blob;

    d = (busd_t *)allocator_mem_alloc(allocator, sizeof(busd_t));
    if (d == NULL) {
        dbg_str(BUS_ERROR, "allocator_mem_alloc");
        return NULL;
    }

    memset(d, 0, sizeof(busd_t));

    blob = blob_create(allocator);
    if (blob == NULL) {
        dbg_str(BUS_WARN, "blob_create");
        return NULL;
    }
    blob_init(blob);
    d->blob = blob;

    d->allocator = allocator;

    return d;
}

int busd_set(busd_t *busd, char *attrib_name, char *value, int value_len)
{
    if (!strcmp(attrib_name, "server_sk_type")) {
        busd->server_sk_type = value;
    }else if (!strcmp(attrib_name, "server_host")) {
        busd->server_host = value;
    }else{
        dbg_str(DBG_WARN, "not support attrib setting, please check");
    }

    return 0;
}

int busd_init(busd_t *busd, 
              char *server_host, 
              char *server_srv, 
              int (*process_server_task_cb)(void *task))
{
    Map *map;
    int value_type = VALUE_TYPE_STRUCT_POINTER;
    int trustee_flag = 1;
    int ret; 

    TRY {
        if (busd->server_sk_type == NULL) {
            busd->server_sk_type = (char *)((SERVER_TYPE_INET_TCP));
        }
        busd->server_host = server_host;
        busd->server_srv  = server_srv;
        dbg_str(BUS_DETAIL, "busd_init, busd:%p", busd);

        busd->server= server(busd->allocator, 
                            busd->server_sk_type, 
                            busd->server_host, 
                            busd->server_srv, 
                            process_server_task_cb, 
                            busd);
        THROW_IF(busd->server == NULL, -1);

        /*create object hash map*/
        busd->obj_map = object_new(busd->allocator, "RBTree_Map", NULL);
        THROW_IF(busd->obj_map == NULL, -1);
        map = busd->obj_map;
        map->set_cmp_func(map, string_key_cmp_func);
        map->set(map, "/Map/value_type", &value_type);
        map->set(map, "/Map/trustee_flag", &trustee_flag);
        map->set(map, "/Map/class_name", "Busd_Object_Struct_Adapter");
    } CATCH (ret) {}

    return ret;
}

struct busd_object *
busd_create_bus_object(busd_t *busd, char *object_id, blob_attr_t *attr, int fd)
{
    struct busd_object *obj;
    blob_attr_t *attrib, *head;
    uint32_t len;
    struct busd_object_method *method; 
    allocator_t *allocator = busd->allocator;
    char *object_infos;

    obj = (struct busd_object *)allocator_mem_zalloc(allocator, 
                                                     sizeof(struct busd_object));
    if (obj == NULL) {
        dbg_str(BUS_ERROR, "allocator_mem_alloc");
        return NULL;
    }

    object_infos   = blob_get_string(attr);
    obj->fd        = fd;
    obj->allocator = allocator;
    strncpy(obj->id, object_id, strlen(object_id));

    obj->infos = (char *)allocator_mem_zalloc(allocator, strlen(object_infos));
    strncpy(obj->infos, object_infos, strlen(object_infos));
    // busd_dump_object(obj);

    return obj;
}

int busd_release_bus_object(busd_t *busd, int fd)
{
    Map *map = busd->obj_map;
    struct busd_object *obj;
    Iterator *cur, *end;
    allocator_t *allocator;
    void *key, *value;
    int ret;

    TRY {
        cur = map->begin(map);
        end = map->end(map);

        for (; !end->equal(end, cur); cur->next(cur)) {
            key = cur->get_kpointer(cur);
            value = cur->get_vpointer(cur);
            obj = (struct busd_object *)value;
            if (obj->fd != fd) continue;
            EXEC(map->remove(map, obj->id, NULL));
            dbg_str(BUS_VIP, "busd release service, object_id:%s, fd:%d", obj->id, fd);
            allocator = obj->allocator;
            allocator_mem_free(allocator, obj->infos);
            allocator_mem_free(allocator, obj);
        }
    } CATCH (ret) {}

    return ret;
}

int busd_reply_add_object(busd_t *busd, int state, char *object_id, int fd)
{
    bus_reqhdr_t hdr;
    blob_t *blob = busd->blob;
    uint8_t buffer[BLOB_BUFFER_MAX_SIZE];
    uint32_t buffer_len;
    allocator_t *allocator = busd->allocator;

    dbg_str(BUS_SUC, "busd_reply_add_object, object_id:%s, state:%d, fd:%d", object_id, state, fd);
    memset(&hdr, 0, sizeof(hdr));

    hdr.type = BUSD_REPLY_ADD_OBJECT;

    blob_reset(blob);
    blob_add_table_start(blob, (char *)"add_obj_reply"); {
        blob_add_uint32(blob, (char *)"state", state);
        blob_add_string(blob, (char *)"object_id", object_id);
    }
    blob_add_table_end(blob);

    memcpy(buffer, &hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);
    memcpy(buffer + buffer_len, (uint8_t *)blob->head,
           blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    dbg_buf(BUS_DETAIL, "busd send:", buffer, buffer_len);

    send(fd, buffer, buffer_len, 0);  

    return 0;
}

int busd_handle_add_object(busd_t *busd, blob_attr_t **attr, int fd)
{
    struct busd_object *obj;
    Map *map = busd->obj_map;
    char *object_id;
    int state = -1;

    if (attr[BUSD_OBJID]) {
        object_id = blob_get_string(attr[BUSD_OBJID]);
    }
    if (attr[BUSD_OBJINFOS]) {
        obj = busd_create_bus_object(busd, blob_get_string(attr[BUSD_OBJID]), 
                                     attr[BUSD_OBJINFOS], fd);
        if (obj != NULL) {
            map->add(map, obj->id, obj);
            state = 1;
        }
    }

    busd_reply_add_object(busd, state, object_id, fd);

    return 0;
}

int busd_reply_lookup_object(busd_t *busd, struct busd_object *obj, int fd)
{
    bus_reqhdr_t hdr;
    blob_t *blob = busd->blob;
    uint8_t buffer[BLOB_BUFFER_MAX_SIZE];
    uint32_t buffer_len;
    allocator_t *allocator = busd->allocator;
    Map *map = busd->obj_map;
    cjson_t *root;
    char *json;
    int ret;

    TRY {
        memset(&hdr, 0, sizeof(hdr));
        hdr.type = BUSD_REPLY_LOOKUP;

        blob_reset(blob);
        blob_add_table_start(blob, (char *)"lookup_reply");
        if (obj == NULL) {
            blob_add_string(blob, (char *)"object_id", "all");
            json = map->to_json(map);
            blob_add_string(blob, (char *)"object_infos", json);
        } else {
            dbg_str(BUS_VIP, "busd_reply_lookup_object, object_id:%s, fd:%d", obj->id, fd);
            blob_add_string(blob, (char *)"object_id", obj->id);
            root = cjson_create_array();
            busd_object_struct_custom_to_json(root, obj);
            json = cjson_print(root);
            blob_add_string(blob, (char *)"object_infos", json);
            cjson_delete(root);
            free(json);
        }
        blob_add_table_end(blob);

        memset(buffer, 0, sizeof(buffer));
        memcpy(buffer, &hdr, sizeof(hdr));
        buffer_len = sizeof(hdr);
        memcpy(buffer + buffer_len, (uint8_t *)blob->head,
            blob_get_len((blob_attr_t *)blob->head));
        buffer_len += blob_get_len((blob_attr_t *)blob->head);

        dbg_buf(BUS_DETAIL, "bus send:", buffer, buffer_len);
        send(fd, buffer, buffer_len, 0);
    } CATCH (ret) {}

    return ret;
}

int busd_handle_lookup_object(busd_t *busd, blob_attr_t **attr, int fd)
{
    struct busd_object *obj = NULL;
    Map *map = busd->obj_map;
    char *key;
    int ret;

    TRY {
        THROW_IF(attr[BUSD_OBJID] == NULL, -1);
        key = blob_get_string(attr[BUSD_OBJID]);
        THROW_IF(key == NULL, -1);

        if (strcmp(key, "all") == 0) {
            dbg_str(BUS_INFO, "busd_handle_lookup_object all");
            busd_reply_lookup_object(busd, NULL, fd);
        } else {
            ret = map->search(map, key, (void **)&obj);
            if (ret > 0) {
                dbg_str(BUS_DETAIL, "obj addr:%p", obj);
                busd_dump_object(obj);
                busd_reply_lookup_object(busd, obj, fd);
            }
        }
    } CATCH (ret) {}

    return ret;
}

int 
busd_forward_invoke(busd_t *busd, int src_fd, int dest_fd,
                    char *object_id, char *method, 
                    int argc, blob_attr_t *args)
{
    bus_reqhdr_t hdr;
    blob_t *blob = busd->blob;
    uint8_t buffer[BLOB_MAX_SIZE];
    uint32_t buffer_len;
    allocator_t *allocator = busd->allocator;

    dbg_str(BUS_VIP, "busd_forward_invoke, object_id:%s, method:%s, source_fd:%d, dest_fd:%d", object_id, method, src_fd, dest_fd);
    memset(&hdr, 0, sizeof(hdr));

    hdr.type = BUSD_FORWARD_INVOKE;

    blob_reset(blob);
    blob_add_table_start(blob, (char *)"forword_invoke"); {
        blob_add_uint32(blob, (char *)"source_fd", src_fd);
        blob_add_uint32(blob, (char *)"destination_fd", dest_fd);
        blob_add_string(blob, (char *)"object_id", object_id);
        blob_add_string(blob, (char *)"invoke_method", method);
        blob_add_uint8(blob, (char *)"invoke_argc", argc);
        blob_catenate(blob, args);
    }
    blob_add_table_end(blob);

    memcpy(buffer, &hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);
    memcpy(buffer + buffer_len, (uint8_t *)blob->head, 
           blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    dbg_buf(BUS_DETAIL, "busd send:", buffer, buffer_len);

    send(dest_fd, buffer, buffer_len, 0);  

    return 0;
}

int busd_handle_invoke_method(busd_t *busd, blob_attr_t **attr, int fd)
{
    struct busd_object *obj = NULL;
    Map *map = busd->obj_map;
    char *key;
    uint8_t *p = NULL;
    char *method = NULL;
    char *object_id = NULL;
    blob_attr_t *args = NULL;
    int argc = 0; 
    int ret;

    TRY {
        if (attr[BUSD_OBJID]) {
            object_id = blob_get_string(attr[BUSD_OBJID]);
        }
        if (attr[BUSD_INVOKE_METHORD]) {
            method = blob_get_string(attr[BUSD_INVOKE_METHORD]);
        }
        if (attr[BUSD_INVOKE_ARGC]) {
            argc = blob_get_uint8(attr[BUSD_INVOKE_ARGC]); 
        }
        if (attr[BUSD_INVOKE_ARGS]) {
            args = attr[BUSD_INVOKE_ARGS];
        }

        THROW_IF(object_id == NULL, -1);
        ret = map->search(map, object_id, (void **)&obj);
        THROW_IF(ret <= 0, -1);

        EXEC(busd_forward_invoke(busd, fd, obj->fd, object_id, method, argc, args));
    } CATCH (ret) {
        busd_reply_invoke(busd, object_id, method, BUS_RET_FAIL, NULL, 0, fd, fd);
        dbg_str(BUS_ERROR, "busd_handle_invoke_method error, src fd:%d, dest fd:%d, object:%s, method:%s", 
                fd, obj != NULL ? obj->fd : 0, object_id != NULL ? object_id : "na", method != NULL ? method : "na");
    }

    return ret;
}

int busd_reply_invoke(busd_t *busd, char *object_id, char *method, int state, uint8_t *opaque, int opaque_len, int source_fd, int dest_fd)
{
    bus_reqhdr_t hdr;
    blob_t *blob = busd->blob;
    uint8_t buffer[BLOB_BUFFER_MAX_SIZE];
    uint32_t buffer_len;
    allocator_t *allocator = busd->allocator;

    dbg_str(BUS_INFO, "busd_reply_invoke, object_id:%s, method:%s, state:%d, source_fd:%d, dest_fd:%d", 
            object_id, method, state, source_fd, dest_fd);

    memset(&hdr, 0, sizeof(hdr));

    hdr.type = BUSD_REPLY_INVOKE;

    blob_reset(blob);
    blob_add_table_start(blob, (char *)"invoke_reply"); {
        blob_add_uint32(blob, (char *)"state", state);
        blob_add_buffer(blob, (char *)"opaque", opaque, opaque_len);
        blob_add_string(blob, (char *)"object_id", object_id);
        blob_add_string(blob, (char *)"invoke_method", method);
    }
    blob_add_table_end(blob);

    memcpy(buffer, &hdr, sizeof(hdr));
    buffer_len = sizeof(hdr);
    memcpy(buffer + buffer_len, (uint8_t *)blob->head, 
           blob_get_len((blob_attr_t *)blob->head));
    buffer_len += blob_get_len((blob_attr_t *)blob->head);

    dbg_buf(BUS_DETAIL, "busd send:", buffer, buffer_len);

    send(dest_fd, buffer, buffer_len, 0);  

    return 0;
}

int busd_handle_forward_invoke_reply(busd_t *busd, blob_attr_t **attr, int fd)
{
    int state;
    char *method;
    char *object_id;
    int src_fd;
    uint8_t *buffer = NULL;
    int buffer_len = 0;

    if (attr[BUSD_STATE]) {
        state = blob_get_uint32(attr[BUSD_STATE]); 
    }
    if (attr[BUSD_INVOKE_METHORD]) {
        method = blob_get_string(attr[BUSD_INVOKE_METHORD]); 
    }
    if (attr[BUSD_OBJID]) {
        object_id = blob_get_string(attr[BUSD_OBJID]); 
    }
    if (attr[BUSD_INVOKE_SRC_FD]) {
        src_fd = blob_get_uint32(attr[BUSD_INVOKE_SRC_FD]); 
    }
    if (attr[BUSD_OPAQUE]) {
        buffer_len = blob_get_buffer(attr[BUSD_OPAQUE], &buffer);
    }

    busd_reply_invoke(busd, object_id, method, state, buffer, buffer_len, fd, src_fd);

    /* 如果该req有多个回复，如果cli结束需要通知service结束该req */
    if (state = BUS_RET_NEED_END_NOTIFY) {
    } else if (state == BUS_RET_END_AND_CLEAR_NOTIFY) {
    }

    return 0;
}

int busd_handle_ping(busd_t *busd, blob_attr_t **attr, int fd)
{
    struct busd_object *obj;
    Map *map = busd->obj_map;
    blob_t *blob = busd->blob;
    uint32_t time;
    bus_reqhdr_t hdr;
    uint8_t buffer[BLOB_BUFFER_MAX_SIZE];
    uint32_t buffer_len;
    int ret;

    TRY {
        THROW_IF(attr[BUSD_TIME] == NULL, -1);

        memset(&hdr, 0, sizeof(hdr));
        hdr.type = BUSD_REPLY_PONG;

        time = blob_get_uint32(attr[BUSD_TIME]);
        blob_reset(blob);
        blob_add_table_start(blob, (char *)"TTL"); {
            blob_add_uint32(blob, (char *)"time", time);
        }
        blob_add_table_end(blob);

        memcpy(buffer, &hdr, sizeof(hdr));
        buffer_len = sizeof(hdr);
        memcpy(buffer + buffer_len, (uint8_t *)blob->head,
            blob_get_len((blob_attr_t *)blob->head));
        buffer_len += blob_get_len((blob_attr_t *)blob->head);

        dbg_buf(BUS_DETAIL, "busd send:", buffer, buffer_len);

        send(fd, buffer, buffer_len, 0); 
    } CATCH (ret) {}

    return ret;
}

static busd_cmd_callback handlers[__BUS_REQ_LAST] = {
    [BUS_REQ_ADD_OBJECT]       = busd_handle_add_object, 
    [BUS_REQ_LOOKUP]           = busd_handle_lookup_object, 
    [BUS_REQ_INVOKE]           = busd_handle_invoke_method, 
    [BUS_REPLY_FORWARD_INVOKE] = busd_handle_forward_invoke_reply, 
    [BUS_REQ_PING]             = busd_handle_ping,
};

static int busd_process_receiving_data_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    bus_reqhdr_t *hdr;
    blob_attr_t *blob_attr;
    blob_attr_t *tb[__BUSD_MAX];
    busd_cmd_callback cb = NULL;
    busd_t *busd = (busd_t *)t->opaque;
    allocator_t *allocator = busd->allocator;
    Ring_Buffer *rb;
    char buffer[BLOB_MAX_SIZE];
    int len, buffer_len, blob_table_len = 0, ret;

    TRY {
        THROW_IF(t->buf_len <= 0, 0);
        if (t->buf_len > 2 && ((char *)(t->buf))[0] == 0 && ((char *)(t->buf))[1] != BUS_REQ_PING) {
            dbg_buf(BUS_DETAIL, "busd receive:", (uint8_t *)t->buf, t->buf_len);
        }

        /* 1.将数据写入cache */
        if (t->cache == NULL) {
            t->cache = object_new(allocator, "Ring_Buffer", NULL);
            rb = t->cache;
            rb->set_size(rb, BLOB_MAX_SIZE);
            dbg_str(BUS_DETAIL, "new cache fd:%d", t->fd);
        } else {
            rb = t->cache;
        }
        
        rb->write(rb, t->buf, t->buf_len);
        buffer_len = rb->get_len(rb);

        do {
            /* 2.判断数据头 */
            THROW_IF(buffer_len < sizeof(bus_reqhdr_t) + sizeof(blob_attr_t), 0); //数据小于req和table头，返回等待下一次读信号。
            rb->peek(rb, buffer, sizeof(bus_reqhdr_t));  //数据可能没有收齐，所以需要先peek一下
            hdr = (bus_reqhdr_t *)buffer;
            THROW_IF(hdr->type > __BUS_REQ_LAST || hdr->version != 0 || hdr->s_id != 0 || hdr->d_id != 0, -1);
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
            EXEC(blob_parse_to_attr(busd_attribs, 
                                    ARRAY_SIZE(busd_attribs), 
                                    tb,
                                    blob_attr, 
                                    len));

            /* 6.调用相应回调处理数据 */
            cb = handlers[hdr->type];
            THROW_IF(cb == NULL, -1);
            EXEC(cb(busd, tb, t->fd));
            buffer_len = rb->get_len(rb);
            dbg_str(BUS_DETAIL, "left buffer_len:%d", buffer_len);
        } while (buffer_len > 0);
    } CATCH (ret) { 
        dbg_str(BUS_ERROR, "busd_process_receiving_data_callback error, task fd:%d, buf_len:%d", t->fd, t->buf_len);
    } FINALLY {
        if (t->buf_len <= 0 || ret < 0) {
            object_destroy(t->cache);
            t->cache = NULL;
            dbg_str(BUS_DETAIL, "close cache fd:%d", t->fd);
        }

        /* 检测到bus service异常，需要释放bus obj */
        if (t->buf_len <= 0) {
            busd_release_bus_object(busd, t->fd);
        }
    }

    return ret;
}

busd_t *busd_create(allocator_t *allocator, 
                    char *server_host, 
                    char *server_srv, 
                    char *socket_type)
{
    busd_t *busd = NULL;
    int ret;
    
    TRY {
        dbg_str(BUS_DETAIL, "bus_daemon_create");
        busd = busd_alloc(allocator);

        EXEC(busd_set(busd, (char *)"server_sk_type", socket_type, 0));
        EXEC(busd_init(busd, //busd_t *busd, 
                       server_host, //char *server_host, 
                       server_srv, //char *server_srv, 
                       busd_process_receiving_data_callback));
    } CATCH (ret) {
        busd_destroy(busd);
        busd = NULL;
    }

    return busd;
}

int busd_destroy(busd_t *busd)
{
    allocator_t *allocator;
    Map *map;
    int ret;

    TRY {
        THROW_IF(busd == NULL, 0);
        dbg_str(BUS_VIP, "destory busd!");

        allocator = busd->allocator;
        map = busd->obj_map;

        if (busd->server) server_destroy(busd->server);
        blob_destroy(busd->blob);

        if (map != NULL) {
            map->for_each(map, busd_release_object);
            object_destroy(busd->obj_map);
        }
        
        allocator_mem_free(allocator, busd);
    } CATCH (ret) {}

    return ret;
}