/**
 * @file Akcipher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-04-25
 */

#include <libobject/crypto/Akcipher.h>

static class_info_entry_t ak_class_info[] = {
    Init_Obj___Entry(0, Obj,      parent),
    Init_Nfunc_Entry(1, Akcipher, construct, NULL),
    Init_Nfunc_Entry(2, Akcipher, deconstruct, NULL),
    Init_Vfunc_Entry(3, Akcipher, generate_keys, NULL),
    Init_Vfunc_Entry(4, Akcipher, get_public_key, NULL),
    Init_Vfunc_Entry(5, Akcipher, get_private_key, NULL),
    Init_Vfunc_Entry(6, Akcipher, encrypt, NULL),
    Init_Vfunc_Entry(7, Akcipher, decrypt, NULL),
    Init_Vfunc_Entry(8, Akcipher, compare_keys, NULL),
    Init_End___Entry(9, Akcipher),
};
REGISTER_CLASS(Akcipher, ak_class_info);


