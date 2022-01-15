
/**
 * @file Turn_Client.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Turn_Client.h"

static int __construct(Turn_Client *turn, char *init_str)
{
    allocator_t *allocator = turn->parent.allocator;
    int ret = 0;

    TRY {
       turn->req = object_new(allocator, "Request", NULL);
       turn->response = object_new(allocator, "Response", NULL);
       THROW_IF(turn->req == NULL || turn->response == NULL, -1);

    } CATCH (ret) {
    }
    
    return ret;
}

static int __deconstruct(Turn_Client *turn)
{
    object_destroy(turn->req);
    object_destroy(turn->response);
    return 0;
}


static int __set_read_post_callback(Turn_Client *turn, int (*func)(Response *, void *arg)) 
{
    turn->response->read_post_callback = func;

    return 1;
}

static class_info_entry_t turn_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Turn_Client, construct, __construct),
    Init_Nfunc_Entry(2, Turn_Client, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Turn_Client, connect, NULL),
    Init_Vfunc_Entry(4, Turn_Client, allocate_address, NULL),
    Init_Vfunc_Entry(5, Turn_Client, send, NULL),
    Init_Vfunc_Entry(6, Turn_Client, set_read_post_callback, __set_read_post_callback),
    Init_End___Entry(7, Turn_Client),
};
REGISTER_CLASS("Turn_Client", turn_class_info);
