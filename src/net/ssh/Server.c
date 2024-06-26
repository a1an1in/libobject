/**
 * @file SSH_Server.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/net/ssh/Server.h>

static int __construct(SSH_Server *server, char *init_str)
{
    return 0;
}

static int __deconstruct(SSH_Server *server)
{
    return 0;
}

static class_info_entry_t server_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, SSH_Server, construct, __construct),
    Init_Nfunc_Entry(2, SSH_Server, deconstruct, __deconstruct),
    Init_End___Entry(3, SSH_Server),
};
REGISTER_CLASS(SSH_Server, server_class_info);

