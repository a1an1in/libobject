/**
 * @file Digest.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/crypto/Digest.h>

static int __construct(Digest *digest, char *init_str)
{
    return 0;
}

static int __deconstruct(Digest *digest)
{
    return 0;
}

static class_info_entry_t digest_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Digest, construct, __construct),
    Init_Nfunc_Entry(2, Digest, deconstruct, __deconstruct),
    Init_End___Entry(3, Digest),
};
REGISTER_CLASS("Digest", digest_class_info);

