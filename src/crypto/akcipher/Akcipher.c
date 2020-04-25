/**
 * @file Akcipher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-04-25
 */

#include <libobject/crypto/Akcipher.h>

static int __construct(Akcipher *ak, char *init_str)
{
    return 0;
}

static int __deconstruct(Akcipher *ak)
{
    return 0;
}

static class_info_entry_t ak_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Akcipher, construct, __construct),
    Init_Nfunc_Entry(2, Akcipher, deconstruct, __deconstruct),
    Init_End___Entry(3, Akcipher),
};
REGISTER_CLASS("Akcipher", ak_class_info);

