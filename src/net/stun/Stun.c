
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
    int ret = 0;

    TRY {
       stun->req = object_new(allocator, "Request", NULL);
       stun->response = object_new(allocator, "Response", NULL);
       THROW_IF(stun->req == NULL || stun->response == NULL, -1);

    } CATCH (ret) {
    }
    
    return ret;
}

static int __deconstruct(Stun *stun)
{
    object_destroy(stun->req);
    object_destroy(stun->response);
    return 0;
}

static class_info_entry_t stun_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Stun, construct, __construct),
    Init_Nfunc_Entry(2, Stun, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Stun, connect, NULL),
    Init_Vfunc_Entry(4, Stun, discovery, NULL),
    Init_Vfunc_Entry(5, Stun, send, NULL),
    Init_End___Entry(6, Stun),
};
REGISTER_CLASS("Stun", stun_class_info);

