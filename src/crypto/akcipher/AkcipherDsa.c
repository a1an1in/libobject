/**
 * @file AkcipherDsa.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-07-25
 */

#include "AkcipherDsa.h"

static int __construct(AkcipherDsa *cipher)
{
    return 0;
}

static int __deconstruct(AkcipherDsa *cipher)
{
    return 0;
}

 static int __generate_keys(AkcipherDsa *cipher, int key_len)
 {

 }

 static int __get_public_key(AkcipherDsa *cipher)
 {

 }

 static int __get_private_key(AkcipherDsa *cipher)
 {

 }

 static int __encrypt(AkcipherDsa *cipher, void *key, const u8 *in, const u32 in_len, u8 *out, u32 *out_len)
 {
    dbg_str(DBG_DETAIL, "AkcipherDsa encrypt");
 }

 static int __decrypt(AkcipherDsa *cipher, void *key, const u8 *in, const u32 in_len, u8 *out, u32 *out_len)
 {

 }

static class_info_entry_t cipher_algo_class_info[] = {
    Init_Obj___Entry(0 , Akcipher, parent),
    Init_Nfunc_Entry(1 , AkcipherDsa, construct, __construct),
    Init_Nfunc_Entry(2 , AkcipherDsa, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , AkcipherDsa, generate_keys, __generate_keys),
    Init_Vfunc_Entry(4 , AkcipherDsa, get_public_key, __get_public_key),
    Init_Vfunc_Entry(5 , AkcipherDsa, get_private_key, __get_private_key),
    Init_Vfunc_Entry(6 , AkcipherDsa, encrypt, __encrypt),
    Init_Vfunc_Entry(7 , AkcipherDsa, decrypt, __decrypt),
    Init_End___Entry(8 , AkcipherDsa),
};
REGISTER_CLASS("AkcipherDsa", cipher_algo_class_info);
