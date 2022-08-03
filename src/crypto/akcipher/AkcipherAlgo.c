/**
 * @file AkcipherAlgo.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-07-25
 */

#include <libobject/crypto/AkcipherAlgo.h>


static int __construct(AkcipherAlgo *algo)
{
    algo->padding = AKCIPHER_PADDING_PKCS7;

    return 0;
}

static int __deconstruct(AkcipherAlgo *algo)
{
    object_destroy(algo->sub_algo);

    return 0;
}

static int __pad(AkcipherAlgo *algo, const u8 *in, const u32 in_len)
{
}

static int __unpad(AkcipherAlgo *algo, u8 *out, u32 *out_len)
{

}

static class_info_entry_t cipher_algo_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , AkcipherAlgo, construct, __construct),
    Init_Nfunc_Entry(2 , AkcipherAlgo, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , AkcipherAlgo, set_key, NULL),
    Init_Vfunc_Entry(4 , AkcipherAlgo, encrypt, NULL),
    Init_Vfunc_Entry(5 , AkcipherAlgo, decrypt, NULL),
    Init_Vfunc_Entry(6 , AkcipherAlgo, pad, __pad),
    Init_Vfunc_Entry(7 , AkcipherAlgo, unpad, __unpad),
    Init_Vfunc_Entry(8 , AkcipherAlgo, get_block_size, NULL),
    Init_End___Entry(9 , AkcipherAlgo),
};
REGISTER_CLASS("AkcipherAlgo", cipher_algo_class_info);
