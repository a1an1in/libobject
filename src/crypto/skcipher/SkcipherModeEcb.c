/**
 * @file SkcipherModeEcb.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-07-25
 */

#include <libobject/crypto/SkcipherModeEcb.h>
#include <libobject/crypto/Skcipher.h>

static int __encrypt(SkcipherAlgo *algo, const u8 *in, const u32 in_len, u8 *out, u32 *out_len)
{
    SkcipherAlgo *sub_algo;
    CRIPTO_FUNC func;
    int ret, i, block_size;

    TRY {
        sub_algo = algo->sub_algo;
        func = sub_algo->encrypt;
        EXEC(algo->get_block_size(algo, &block_size));

        dbg_str(DBG_DETAIL, "algo padding:%d, size=%d", algo->padding, block_size);

        for (i = 0; i < in_len && in_len - i >= block_size; i += block_size) {
            EXEC(func(sub_algo, in, block_size, out, 16));
            in += block_size;
            out += block_size;
        }

        EXEC(algo->pad(algo, in, in_len - i));
        if (algo->padding != SKCIPHER_PADDING_NO) {
            EXEC(func(sub_algo, algo->last_block, block_size, out, 16));
        }

        *out_len = i + block_size;
    } CATCH (ret) {
    }

    return ret;
}

static int __decrypt(SkcipherAlgo *algo, const u8 *in, const u32 in_len, u8 *out, u32 *out_len)
{
    SkcipherAlgo *sub_algo;
    CRIPTO_FUNC func;
    int ret, i, block_size;
    u8 *out_head_addr = out;

    TRY {
        sub_algo = algo->sub_algo;
        func = sub_algo->decrypt;
        EXEC(algo->get_block_size(algo, &block_size));

        for (i = 0; i < in_len; i += block_size) {
            THROW_IF(i > *out_len, -1);
            EXEC(func(sub_algo, in, block_size, out, block_size));
            in += block_size;
            out += block_size;
        }

        *out_len = in_len;
        if (algo->padding != SKCIPHER_PADDING_NO) {
            EXEC(algo->unpad(algo, out_head_addr, out_len));
        }
    } CATCH (ret) {
    }

    return ret;
}

static int __set_key(SkcipherAlgo *algo, char *in_key, unsigned int key_len)
{
    SkcipherAlgo *sub_algo = algo->sub_algo;

    return TRY_EXEC(sub_algo->set_key(sub_algo, in_key, key_len));
}

static int __get_block_size(SkcipherAlgo *algo, u32 *size)
{
    SkcipherAlgo *sub_algo = algo->sub_algo;

    return TRY_EXEC(sub_algo->get_block_size(sub_algo, size));
}

static int __create(SkcipherModeEcb *mode, char *sub_algo_name, SkcipherAlgo *algo)
{
    allocator_t *allocator = mode->parent.parent.allocator;
    SkcipherAlgo *sub_algo;
    int ret;

    TRY {
        THROW_IF(algo == NULL || mode == NULL || sub_algo == NULL, -1);

        sub_algo = object_new(allocator, sub_algo_name, NULL);
        THROW_IF(sub_algo == NULL, -1);
        algo->sub_algo = sub_algo;

        algo->encrypt = __encrypt;
        algo->decrypt = __decrypt;
        algo->set_key = __set_key;
        algo->get_block_size = __get_block_size;

        dbg_str(DBG_DETAIL, "SkcipherModeEcb create");
    } CATCH (ret) {
    }

    return ret;
}

static class_info_entry_t cipher_mode_ecb_class_info[] = {
    Init_Obj___Entry(0, SkcipherMode, parent),
    Init_Nfunc_Entry(1, SkcipherModeEcb, construct, NULL),
    Init_Nfunc_Entry(2, SkcipherModeEcb, deconstruct, NULL),
    Init_Vfunc_Entry(3, SkcipherModeEcb, create, __create),
    Init_End___Entry(4, SkcipherModeEcb),
};
REGISTER_CLASS("Ecb", cipher_mode_ecb_class_info);