/**
 * @file SkcipherModeEcb.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-07-25
 */

#include <libobject/crypto/SkcipherModeEcb.h>
#include <libobject/crypto/Skcipher.h>

static int __encrypt(CipherAlgo *algo, const u8 *in, const u32 in_len, u8 *out, u32 out_len)
{
    CipherAlgo *sub_algo;
    CRIPTO_FUNC func;
    int ret, i;

    sub_algo = algo->sub_algo;
    func = sub_algo->encrypt;

    dbg_str(DBG_DETAIL, "algo padding:%d", algo->padding);

    for (i = 0; i < in_len; i += 16) {
        func(sub_algo, in, 16, out, 16);
        in += 16;
        out += 16;
    }

    return ret;
}

static int __decrypt(CipherAlgo *algo, const u8 *in, const u32 in_len, u8 *out, u32 out_len)
{
    CipherAlgo *sub_algo;
    CRIPTO_FUNC func;
    int ret, i;

    sub_algo = algo->sub_algo;
    func = sub_algo->decrypt;

    for (i = 0; i < in_len; i += 16) {
        func(sub_algo, in, 16, out, 16);
        in += 16;
        out += 16;
    }

    return ret;
}

static int __set_key(CipherAlgo *algo, char *in_key, unsigned int key_len)
{
    CipherAlgo *sub_algo = algo->sub_algo;

    return TRY_EXEC(sub_algo->set_key(sub_algo, in_key, key_len));
}

static int __create(SkcipherModeEcb *mode, char *sub_algo_name, CipherAlgo *algo)
{
    allocator_t *allocator = mode->parent.parent.allocator;
    CipherAlgo *sub_algo;
    int ret;

    TRY {
        THROW_IF(algo == NULL || mode == NULL || sub_algo == NULL, -1);

        sub_algo = object_new(allocator, sub_algo_name, NULL);
        THROW_IF(sub_algo == NULL, -1);
        algo->sub_algo = sub_algo;

        algo->encrypt = __encrypt;
        algo->decrypt = __decrypt;
        algo->set_key = __set_key;

        dbg_str(DBG_DETAIL, "SkcipherModeEcb create");
    } CATCH (ret) {
    }

    return ret;
}

static class_info_entry_t cipher_mode_ecb_class_info[] = {
    Init_Obj___Entry(0, CipherMode, parent),
    Init_Nfunc_Entry(1, SkcipherModeEcb, construct, NULL),
    Init_Nfunc_Entry(2, SkcipherModeEcb, deconstruct, NULL),
    Init_Vfunc_Entry(3, SkcipherModeEcb, create, __create),
    Init_End___Entry(4, SkcipherModeEcb),
};
REGISTER_CLASS("Ecb", cipher_mode_ecb_class_info);