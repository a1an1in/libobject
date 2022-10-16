/**
 * @file SSH_Client.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/net/ssh/Client.h>

static int __construct(SSH_Client *client, char *init_str)
{
    return 0;
}

static int __deconstruct(SSH_Client *client)
{
    return 0;
}

static class_info_entry_t client_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, SSH_Client, construct, __construct),
    Init_Nfunc_Entry(2, SSH_Client, deconstruct, __deconstruct),
    Init_End___Entry(3, SSH_Client),
};
REGISTER_CLASS("SSH_Client", client_class_info);

