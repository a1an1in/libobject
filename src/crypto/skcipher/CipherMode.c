/**
 * @file CipherMode.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-04-25
 */

#include <libobject/crypto/CipherMode.h>

static class_info_entry_t cipher_algo_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, CipherMode, construct, NULL),
    Init_Nfunc_Entry(2, CipherMode, deconstruct, NULL),
    Init_Vfunc_Entry(3, CipherMode, create, NULL),
    Init_End___Entry(4, CipherMode),
};
REGISTER_CLASS("CipherMode", cipher_algo_class_info);