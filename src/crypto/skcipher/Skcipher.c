/**
 * @file Skcipher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-04-25
 */

#include <libobject/crypto/Skcipher.h>

static int __construct(Skcipher *sk, char *init_str)
{
    return 0;
}

static int __deconstruct(Skcipher *sk)
{
    return 0;
}

static class_info_entry_t sk_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Skcipher, construct, __construct),
    Init_Nfunc_Entry(2, Skcipher, deconstruct, __deconstruct),
    Init_End___Entry(3, Skcipher),
};
REGISTER_CLASS("Skcipher", sk_class_info);

