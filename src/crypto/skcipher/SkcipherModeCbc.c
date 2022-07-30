/**
 * @file SkcipherModeCbc.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-07-29
 */

#include <libobject/crypto/SkcipherModeCbc.h>
#include <libobject/crypto/Skcipher.h>

static int __encrypt(SkcipherAlgo *algo, const u8 *in, const u32 in_len, u8 *out, u32 *out_len)
{
    SkcipherAlgo *sub_algo;
    CRIPTO_FUNC func;
    uint8_t *iv;
    int ret, i, j, block_size;

    TRY {
        sub_algo = algo->sub_algo;
        func = sub_algo->encrypt;
        iv = sub_algo->iv;

        EXEC(algo->get_block_size(algo, &block_size));
        dbg_buf(DBG_DETAIL, "iv:", iv, block_size);
        dbg_str(DBG_DETAIL, "algo padding:%d, size=%d", algo->padding, block_size);

        for (i = 0; i < in_len && in_len - i >= block_size; i += block_size) {
            for (j = 0; j < block_size; j++) {
                out[j] = in[j] ^ iv[j];
            }
            EXEC(func(sub_algo, out, block_size, out, 16));
            iv = out;
            in += block_size;
            out += block_size;
        }

        EXEC(algo->pad(algo, in, in_len - i));
        if (algo->padding != SKCIPHER_PADDING_NO) {
            for (j = 0; j < block_size; j++) {
                out[j] = algo->last_block[j] ^ iv[j];
            }
            EXEC(func(sub_algo, out, block_size, out, 16));
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
    uint8_t *iv, tmp;
    int ret, i, j, block_size;
    u8 *out_head_addr = out;

    TRY {
        THROW_IF(in == out, -1);
        sub_algo = algo->sub_algo;
        func = sub_algo->decrypt;
        iv = sub_algo->iv;
        EXEC(algo->get_block_size(algo, &block_size));

        for (i = 0; i < in_len; i += block_size) {
            THROW_IF(i > *out_len, -1);
            EXEC(func(sub_algo, in, block_size, out, block_size));
            for (j = 0; j < block_size; j++) {
                out[j] = out[j] ^ iv[j];
            }
            iv = in;
            in += block_size;
            out += block_size;
            dbg_buf(DBG_DETAIL, "iv:", iv, 16);
        }

        *out_len = in_len;
        dbg_buf(DBG_DETAIL, "out:", out_head_addr, i);
        if (algo->padding != SKCIPHER_PADDING_NO) {
            EXEC(algo->unpad(algo, out_head_addr, out_len));
        }
        dbg_str(DBG_DETAIL, "out len:%d, in len:%d, blocksize:%d", *out_len, in_len, block_size);
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

static int __set_iv(SkcipherAlgo *algo, void *iv)
{
    SkcipherAlgo *sub_algo = algo->sub_algo;

    return TRY_EXEC(sub_algo->set_iv(sub_algo, iv));
}

static int __create(SkcipherModeCbc *mode, char *sub_algo_name, SkcipherAlgo *algo)
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
        algo->set_iv = __set_iv;

        dbg_str(DBG_DETAIL, "SkcipherModeCbc create");
    } CATCH (ret) {
    }

    return ret;
}

static class_info_entry_t cipher_mode_Cbc_class_info[] = {
    Init_Obj___Entry(0, SkcipherMode, parent),
    Init_Nfunc_Entry(1, SkcipherModeCbc, construct, NULL),
    Init_Nfunc_Entry(2, SkcipherModeCbc, deconstruct, NULL),
    Init_Vfunc_Entry(3, SkcipherModeCbc, create, __create),
    Init_End___Entry(4, SkcipherModeCbc),
};
REGISTER_CLASS("Cbc", cipher_mode_Cbc_class_info);