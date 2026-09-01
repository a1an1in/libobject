
/**
 * @file Request.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <math.h>
#include "Request.h"

static int __construct(Request *request, char *init_str)
{
    allocator_t *allocator = request->parent.allocator;
    int ret = 0, trustee_flag = 1;;
    int value_type = VALUE_TYPE_ALLOC_POINTER;
    Map *map;

   /*
    * for IPv4, the actual STUN message would need
    * to be less than 548 bytes (576 minus 20-byte IP header, minus 8-byte
    * UDP header, assuming no IP options are used). 
    */
    TRY {
        request->header_max_len = 548;
        request->header = allocator_mem_alloc(allocator, request->header_max_len); 
        THROW_IF(request->header == NULL, -1); 
        map = object_new(allocator, "RBTree_Map", NULL);
        THROW_IF(map == NULL, -1);
        map->set_cmp_func(map, default_key_cmp_func);
        map->set(map, "/Map/trustee_flag", &trustee_flag);
        map->set(map, "/Map/value_type", &value_type);
        request->attribs = map;
    } CATCH (ret) {
    }

    return ret;
}

static int __deconstruct(Request *request)
{
    allocator_t *allocator = request->parent.allocator;

    allocator_mem_free(allocator, request->header);
    object_destroy(request->attribs);

    return 0;
}

/*
 * 将属性序列化到 header->attr 后面的字节流，并更新 header->msglen。
 * STUN 属性为 TLV 格式，value 需按 4 字节对齐（不足补 0）。
 * 注意 stun_attrib_t 因为内嵌 union 导致 sizeof 远大于 4，因此
 * 属性头长度必须使用 STUN_ATTR_HEADER_LEN(4)，不能使用 sizeof(stun_attrib_t)。
 */
static int __set_attrib(Request *request, int type, int len, char *value)
{
    stun_header_t *header = request->header;
    char *p;
    uint16_t *type_p, *len_p;
    int ret = 0, offset, padded_len, pad;

    TRY {
        offset = ntohs(header->msglen);
        THROW_IF(offset + STUN_ATTR_HEADER_LEN + len >
                 request->header_max_len - (int)sizeof(stun_header_t), -1);

        p = (char *)header->attr + offset;
        type_p = (uint16_t *)p;
        len_p = (uint16_t *)(p + 2);
        *type_p = htons(type);
        *len_p = htons(len);
        memcpy(p + STUN_ATTR_HEADER_LEN, value, len);

        /* 4 字节对齐 padding */
        padded_len = (len + STUN_ATTR_ALIGN - 1) & ~(STUN_ATTR_ALIGN - 1);
        pad = padded_len - len;
        if (pad > 0) memset(p + STUN_ATTR_HEADER_LEN + len, 0, pad);

        header->msglen = htons(offset + STUN_ATTR_HEADER_LEN + padded_len);
    } CATCH (ret) {
    }

    return ret;
}

static int __set_head(Request *request, int type, int len, uint32_t cookie)
{
    int i = 0;
    int ret = 0;

    TRY {
        request->header->msgtype = htons(type);
        request->header->msglen = htons(len);
        request->header->magic_cookie = htonl(cookie);

        for (i = 0; i < 3; i++) {
#if (defined(WINDOWS_USER_MODE))
            request->header->transaction_id[i] = rand();
#else
            request->header->transaction_id[i] = random();
#endif
        }
    } CATCH (ret) {
    }

    return ret;
}

/* 返回完整报文长度: STUN 头(20字节) + msglen */
static int __get_len(Request *request)
{
    return (int)sizeof(stun_header_t) + ntohs(request->header->msglen);
}

static class_info_entry_t request_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Request, construct, __construct),
    Init_Nfunc_Entry(2, Request, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Request, set_attrib, __set_attrib),
    Init_Vfunc_Entry(4, Request, set_head, __set_head),
    Init_Vfunc_Entry(5, Request, get_len, __get_len),
    Init_End___Entry(6, Request),
};
REGISTER_CLASS(Stun_Request, request_class_info);
