
/**
 * @file Response.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <math.h>
#include "Turn_Client.h"
#include "Response.h"

static int __construct(Response *response, char *init_str)
{
    allocator_t *allocator = response->parent.allocator;
    int ret = 0, trustee_flag = 1;;
    int value_type = VALUE_TYPE_ALLOC_POINTER;

    TRY {
        response->header_max_len = 548;
        response->header = allocator_mem_alloc(allocator, response->header_max_len); 
        THROW_IF(response->header == NULL, -1);

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

    return 0;
}

static int __read_head(Response *response)
{
    turn_header_t * header = response->header;

    header->msgtype = ntohs(header->msgtype);
    header->msglen = ntohs(header->msglen);

    dbg_str(DBG_DETAIL, "response msgtype:%d, msglen:%d", header->msgtype, header->msglen);

    return 0;
}

static int __read_attribs(Response *response)
{
    turn_header_t *header = response->header;
    uint8_t *attr_addr = header->attr;
    turn_attrib_header_t *attr;
    int ret = 0, i = 0, attr_len = 0;
    attrib_parse_policy_t *policies;

    TRY {
        policies = protocol_get_parse_policies();

        for (i = 0; i < header->msglen; ) {
            attr = (turn_attrib_header_t *)(attr_addr + i);
            attr->type = ntohs(attr->type);
            attr->len = ntohs(attr->len);
            attr_len = (sizeof(int) + (attr->len + (4 - (attr->len % 4)) % 4));
            i += attr_len;
            dbg_str(DBG_DETAIL, "attr type :%x , attrib attr len:%d, real len:%d", attr->type, attr->len, attr_len);
            CONTINUE_IF((attr->type > TURN_ATR_TYPE_MAX) || (policies[attr->type].policy == NULL));

            EXEC(policies[attr->type].policy(&response->attribs, attr));
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
REGISTER_CLASS("Turn::Response", response_class_info);

