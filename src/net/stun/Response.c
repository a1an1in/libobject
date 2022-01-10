
/**
 * @file Response.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <math.h>
#include "Response.h"

static int __construct(Response *request, char *init_str)
{
    allocator_t *allocator = request->parent.allocator;
    int ret = 0, trustee_flag = 1;;
    int value_type = VALUE_TYPE_ALLOC_POINTER;
    Map *map;

    TRY {
        request->header = NULL; 
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

static int __deconstruct(Response *request)
{
    allocator_t *allocator = request->parent.allocator;

    allocator_mem_free(allocator, request->header);
    object_destroy(request->attribs);

    return 0;
}

static int __write_attrib(Response *request, int type, int len, char *value)
{
    allocator_t *allocator = request->parent.allocator;
    stun_attrib_t *attrib;
    int ret = 0;

    TRY {
        attrib = allocator_mem_alloc(allocator, sizeof(stun_attrib_t) + len);
        THROW_IF(attrib == NULL, -1);
        
        attrib->len = htons(len);
        attrib->type = htons(type);
        memcpy(attrib->value, value, len);
        request->attribs->add(request->attribs, type, attrib);
    } CATCH (ret) {
    }

    return ret;
}

static int __write_head(Response *request, int type, int len, uint32_t cookie)
{
    int i = 0;
    int ret = 0;

    TRY {
        request->header->msgtype = htons(type);
        request->header->msglen = htons(len);
        request->header->magic_cookie = htonl(cookie);

        for (i = 0; i < 3; i++) {
            request->header->transaction_id[i] = random();
        }
    } CATCH (ret) {
    }

    return ret;
}

static class_info_entry_t request_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Response, construct, __construct),
    Init_Nfunc_Entry(2, Response, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Response, write_attrib, __write_attrib),
    Init_Nfunc_Entry(4, Response, write_head, __write_head),
    Init_End___Entry(5, Response),
};
REGISTER_CLASS("Response", request_class_info);

