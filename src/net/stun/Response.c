
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

static int __read_attribs(Response *response)
{
    allocator_t *allocator = response->parent.allocator;
    stun_header_t *header = response->header;
    uint8_t *attr_addr = header->attr;
    Map *map = response->attribs;
    stun_attrib_t *raw, *attr;
    int ret = 0, i = 0;

    TRY {
        for (i = 0; i < header->msglen; ) {
            raw = (stun_attrib_t *)(attr_addr + i);
            raw->type = ntohs(raw->type);
            raw->len = ntohs(raw->len);
            i +=  sizeof(int) + raw->len;
            dbg_str(DBG_DETAIL, "raw  type :%d , len:%d", raw->type, raw->len);
            CONTINUE_IF((raw->type > STUN_ATR_TYPE_MAX) || (g_parse_attr_policies[raw->type].policy == NULL));

            attr = allocator_mem_alloc(allocator, sizeof(stun_attrib_t));
            EXEC(g_parse_attr_policies[raw->type].policy(raw, attr));
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
    Init_Nfunc_Entry(3, Response, read, __read),
    Init_End___Entry(4, Response),
};
REGISTER_CLASS("Response", response_class_info);

