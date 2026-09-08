
/**
 * @file Response.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <math.h>
#include "Stun.h"
#include "Response.h"

static int __construct(Response *response, char *init_str)
{
    allocator_t *allocator = response->parent.allocator;
    int ret = 0, trustee_flag = 1;;
    int value_type = VALUE_TYPE_ALLOC_POINTER;
    Map *map;

    TRY {
        response->header_max_len = 548;
        response->header = allocator_mem_alloc(allocator, response->header_max_len); 
        THROW_IF(response->header == NULL, -1);

        map = object_new(allocator, "RBTree_Map", NULL);
        THROW_IF(map == NULL, -1);
        map->set_cmp_func(map, default_key_cmp_func);
        map->set(map, "/Map/trustee_flag", &trustee_flag);
        map->set(map, "/Map/value_type", &value_type);
        response->attribs = map;

        response->buffer = object_new(allocator, "Ring_Buffer", NULL);
        THROW_IF(response->buffer == NULL, -1);
    } CATCH (ret) {
    }

    return ret;
}

static int __deconstruct(Response *response)
{
    allocator_t *allocator = response->parent.allocator;

    allocator_mem_free(allocator, response->header);
    object_destroy(response->attribs);
    object_destroy(response->buffer);

    return 0;
}

static int __read_head(Response *response)
{
    stun_header_t * header = response->header;

    header->msgtype = ntohs(header->msgtype);
    header->msglen = ntohs(header->msglen);

    dbg_str(DBG_DETAIL, "response msgtype:%d, msglen:%d", header->msgtype, header->msglen);

    return 0;
}

/*
 * XOR-MAPPED-ADDRESS (RFC 5389) 解码。
 * 对 MAPPED-ADDRESS 的 value 布局做 XOR 逆变换得到真实地址：
 *   port: 与 (magic_cookie >> 16) 异或
 *   IPv4: 与 magic_cookie 的 4 个大端字节异或
 *   IPv6: 与 magic_cookie(96bit) + transaction_id(128bit) 异或
 * header->magic_cookie 保持网络序（未做 ntohl），直接取其大端字节。
 */
static int __xor_decode_mapped_address(stun_header_t *header, stun_attrib_t *attr)
{
    uint8_t *value = (uint8_t *)&attr->u.mapped_address;
    uint8_t family = value[1];
    uint8_t *ip;
    uint16_t port;
    uint32_t cookie_be = header->magic_cookie;
    int i, addr_len;

    memcpy(&port, value + 2, 2);
    port = ntohs(port) ^ (ntohl(header->magic_cookie) >> 16);
    port = htons(port);
    memcpy(value + 2, &port, 2);

    addr_len = (family == 0x01) ? 4 : 16;
    ip = value + 4;
    for (i = 0; i < addr_len; i++) {
        ip[i] ^= ((uint8_t *)&cookie_be)[i % 4];
    }

    return 0;
}

/* 在线性策略表中查找指定属性类型的解析策略 */
static attrib_parse_policy_t *__find_policy(int type)
{
    int i;

    for (i = 0; i < g_stun_parse_attr_policies_count; i++) {
        if (g_stun_parse_attr_policies[i].type == type) {
            return &g_stun_parse_attr_policies[i];
        }
    }

    return NULL;
}

static int __read_attribs(Response *response)
{
    allocator_t *allocator = response->parent.allocator;
    stun_header_t *header = response->header;
    uint8_t *attr_addr = header->attr;
    Map *map = response->attribs;
    stun_attrib_t *raw, *attr;
    attrib_parse_policy_t *policy;
    int ret = 0, i = 0, attr_total;

    TRY {
        for (i = 0; i < header->msglen; ) {
            raw = (stun_attrib_t *)(attr_addr + i);
            raw->type = ntohs(raw->type);
            raw->len = ntohs(raw->len);
            /* 属性按 4 字节对齐，头 4 字节 + 对齐后的 value 长度 */
            attr_total = STUN_ATTR_HEADER_LEN + ((raw->len + STUN_ATTR_ALIGN - 1) & ~(STUN_ATTR_ALIGN - 1));
            i += attr_total;
            dbg_str(DBG_DETAIL, "raw  type :%d , len:%d, total:%d", raw->type, raw->len, attr_total);

            /* XOR-MAPPED-ADDRESS 需要先做 XOR 解码（依赖 header 的 cookie/transaction_id） */
            if (raw->type == STUN_ATR_TYPE_XOR_MAPPED_ADDR) {
                __xor_decode_mapped_address(header, raw);
            }

            policy = __find_policy(raw->type);
            CONTINUE_IF(policy == NULL);

            attr = allocator_mem_alloc(allocator, sizeof(stun_attrib_t));
            EXEC(policy->policy(raw, attr));
            map->add(map, raw->type, attr);
        }
    } CATCH (ret) {
    }

    return ret;
}

static int __read(Response *response)
{
    Ring_Buffer *buffer = response->buffer;
    int len = 0;
    int ret;

    TRY {
        len = buffer->get_len(buffer);
        THROW_IF(len > response->header_max_len || len < 20, -1);

        buffer->read(buffer, response->header, len);

        EXEC(__read_head(response));
        THROW_IF(response->header->msglen == 0, 1);

        EXEC(__read_attribs(response));
    } CATCH (ret) {
    }

    return ret;
}

static class_info_entry_t response_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Response, construct, __construct),
    Init_Nfunc_Entry(2, Response, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Response, read, __read),
    Init_End___Entry(4, Response),
};
REGISTER_CLASS(Stun_Response, response_class_info);
