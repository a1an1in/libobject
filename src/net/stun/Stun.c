
/**
 * @file Stun.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Stun.h"

static int __construct(Stun *stun, char *init_str)
{
    allocator_t *allocator = stun->parent.allocator;
    int ret = 0, trustee_flag = 1;;
    int value_type = VALUE_TYPE_ALLOC_POINTER;
    Map *map;

   /*
    *for IPv4, the actual STUN message would need
    *to be less than 548 bytes (576 minus 20-byte IP header, minus 8-byte
    *UDP header, assuming no IP options are used). 
    */
    TRY {
        THROW_IF(stun->header = allocator_mem_alloc(allocator, 548) == NULL, -1); 
        THROW_IF(map = object_new(allocator, "RBTree_Map", NULL) == NULL, -1);
        map->set_cmp_func(map, string_key_cmp_func);
        map->set(map, "/Map/trustee_flag", &trustee_flag);
        map->set(map, "/Map/value_type", &value_type);
        stun->attribs = map;
    } CATCH (ret) {
    }

    return ret;
}

static int __deconstruct(Stun *stun)
{
    allocator_t *allocator = stun->parent.allocator;

    allocator_mem_free(allocator, stun->header);
    object_destroy(stun->attribs);

    return 0;
}

static int __write_attrib(Stun *stun, char *key, char *value)
{
    allocator_t *allocator = stun->parent.allocator;

    allocator_mem_free(allocator, stun->header);
    object_destroy(stun->attribs);

    return 0;
}

static class_info_entry_t stun_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Stun, construct, __construct),
    Init_Nfunc_Entry(2, Stun, deconstruct, __deconstruct),
    Init_Nfunc_Entry(2, Stun, write_attrib, __write_attrib),
    Init_End___Entry(3, Stun),
};
REGISTER_CLASS("Stun", stun_class_info);

