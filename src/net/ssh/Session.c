/**
 * @file SSH_Session.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/net/ssh/Session.h>

static int __construct(SSH_Session *session, char *init_str)
{
    return 0;
}

static int __deconstruct(SSH_Session *session)
{
    return 0;
}

static class_info_entry_t session_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, SSH_Session, construct, __construct),
    Init_Nfunc_Entry(2, SSH_Session, deconstruct, __deconstruct),
    Init_End___Entry(3, SSH_Session),
};
REGISTER_CLASS(SSH_Session, session_class_info);

