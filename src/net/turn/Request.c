
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
    * for IPv4, the actual TURN message would need
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

        request->buffer = object_new(allocator, "Buffer", NULL);
        THROW_IF(request->buffer == NULL, -1);
        request->buffer->set_capacity(request->buffer, 1024);
    } CATCH (ret) {
    }

    return ret;
}

static int __deconstruct(Request *request)
{
    allocator_t *allocator = request->parent.allocator;

    allocator_mem_free(allocator, request->header);
    object_destroy(request->attribs);
    object_destroy(request->buffer);

    return 0;
}

static int __set_attrib(Request *request, int type, uint8_t *value, int len)
{
    allocator_t *allocator = request->parent.allocator;
    turn_attrib_header_t *attrib;
    int ret = 0;

    TRY {
        attrib = allocator_mem_alloc(allocator, len);
        THROW_IF(attrib == NULL, -1);
        
        memcpy(attrib, value, len);
        request->attribs->add(request->attribs, type, attrib);
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

        srand ((unsigned) time (NULL));
        for (i = 0; i < 3; i++) {
            request->header->transaction_id[i] = random();
        }
    } CATCH (ret) {
    }

    return ret;
}

static class_info_entry_t request_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Request, construct, __construct),
    Init_Nfunc_Entry(2, Request, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Request, set_attrib, __set_attrib),
    Init_Nfunc_Entry(4, Request, set_head, __set_head),
    Init_End___Entry(5, Request),
};
REGISTER_CLASS("Turn::Request", request_class_info);

