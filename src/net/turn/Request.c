
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
    Vector *vector;

   /*
    * for IPv4, the actual TURN message would need
    * to be less than 548 bytes (576 minus 20-byte IP header, minus 8-byte
    * UDP header, assuming no IP options are used). 
    */
    TRY {
        request->header_max_len = 548;
        request->header = allocator_mem_alloc(allocator, request->header_max_len); 
        THROW_IF(request->header == NULL, -1); 
        vector = object_new(allocator, "Vector", NULL);
        THROW_IF(vector == NULL, -1);
        vector->set(vector, "/Vector/trustee_flag", &trustee_flag);
        vector->set(vector, "/Vector/value_type", &value_type);
        request->attribs = vector;

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

static int __set_attrib(Request *request, uint8_t *value)
{
    int ret = 0;

    TRY {
        THROW_IF(value == NULL, -1);
        request->attribs->add_back(request->attribs, value);
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

