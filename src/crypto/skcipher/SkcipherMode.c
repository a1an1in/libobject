/**
 * @file SkcipherMode.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-07-25
 */

#include <libobject/crypto/SkcipherMode.h>

static class_info_entry_t cipher_algo_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, SkcipherMode, construct, NULL),
    Init_Nfunc_Entry(2, SkcipherMode, deconstruct, NULL),
    Init_Vfunc_Entry(3, SkcipherMode, create, NULL),
    Init_End___Entry(4, SkcipherMode),
};
REGISTER_CLASS("SkcipherMode", cipher_algo_class_info);