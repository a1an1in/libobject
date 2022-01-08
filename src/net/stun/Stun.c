
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
    return 0;
}

static int __deconstruct(Stun *stun)
{
    return 0;
}

static class_info_entry_t stun_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Stun, construct, __construct),
    Init_Nfunc_Entry(2, Stun, deconstruct, __deconstruct),
    Init_End___Entry(3, Stun),
};
REGISTER_CLASS("Stun", stun_class_info);

