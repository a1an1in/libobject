/**
 * @file Skcipher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-04-25
 */

#include <libobject/crypto/Skcipher.h>
#include <libobject/encoding/base64.h>

static int __construct(Skcipher *sk, char *init_str)
{
    allocator_t *allocator = sk->parent.allocator;

    sk->algo = object_new(allocator, "CipherAlgo", NULL);

    return 0;
}

static int __deconstruct(Skcipher *sk)
{
    object_destroy(sk->str);
    object_destroy(sk->algo);
    object_destroy(sk->mode);

    return 0;
}

static int __set_algo(Skcipher *sk, char *algo)
{
    allocator_t *allocator = sk->parent.allocator;
    String *str;
    int ret, start = -1, len;
    char mode[MAX_MODE_NAME_LEN] = { 0 };
    char sub_algo[MAX_SUB_ALGO_NAME] = { 0 };

    TRY {
        THROW_IF(algo == NULL, -1);
        str = object_new(allocator, "String", algo);

        sk->str = str;
        ret = str->get_substring(str, "\\((.*?)\\)", 0, &start, &len);
        if (ret >= 0) {
            strncpy(sub_algo, &str->value[start], len);
            strncpy(mode, str->value, start - 1);
            dbg_str(DBG_DETAIL, "find mode:%s, sub_algo:%s", mode, sub_algo);
        } else {
            return -1; // not support now;
        }

        sk->mode = object_new(allocator, mode, NULL);
        THROW_IF(sk->mode == NULL, -1);

        EXEC(sk->mode->create(sk->mode, sub_algo, sk->algo));
    } CATCH (ret) {
    }

    return ret;
}

static int __set_padding(Skcipher *sk, SkcipherPaddingEnum padding)
{
    sk->algo->padding = padding;

    return 0;
}

static int __encrypt(Skcipher *sk, const u8 *in, const u32 in_len, u8 *out, u32 *out_len)
{
    return TRY_EXEC(sk->algo->encrypt(sk->algo, in, in_len, out, out_len));
}

static int __decrypt(Skcipher *sk, const u8 *in, const u32 in_len, u8 *out, u32 *out_len)
{
    return TRY_EXEC(sk->algo->decrypt(sk->algo, in, in_len, out, out_len));
}

static int __set_key(Skcipher *sk, char *in_key, unsigned int key_len)
{
    return TRY_EXEC(sk->algo->set_key(sk->algo, in_key, key_len));
}

static class_info_entry_t sk_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Skcipher, construct, __construct),
    Init_Nfunc_Entry(2, Skcipher, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Skcipher, set_algo, __set_algo),
    Init_Vfunc_Entry(4, Skcipher, set_key, __set_key),
    Init_Vfunc_Entry(5, Skcipher, encrypt, __encrypt),
    Init_Vfunc_Entry(6, Skcipher, decrypt, __decrypt),
    Init_Vfunc_Entry(7, Skcipher, set_padding, __set_padding),
    Init_End___Entry(8, Skcipher),
};
REGISTER_CLASS("Skcipher", sk_class_info);

static int
test_skcipher_ecb_aes(TEST_ENTRY *entry, void *argc, void *argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_alloc();
    Skcipher *sk;
	char *key = "123456";
	char *in = "abcdefghijklmnopq";
	char out[64] = {0}; // 128
    char out_base64[512] = {0}; 
    int out_len = 64, out_base64_len = 512;

    TRY {
        dbg_str(DBG_SUC, "test_skciphter_ecb_aes in");
        sk = object_new(allocator, "Skcipher", NULL);

        EXEC(sk->set_algo(sk, "Ecb(Aes)"));
        EXEC(sk->set_key(sk, key, strlen(key)));
        // EXEC(sk->set_padding(sk, SKCIPHER_PADDING_ZERO));
        EXEC(sk->encrypt(sk, in, strlen(in), out, &out_len));
        dbg_buf(DBG_DETAIL, "in:", in, strlen(in));
        dbg_buf(DBG_DETAIL, "Ecb(Aes) encode:", out, out_len);
     
        EXEC(base64_encode(out, out_len, out_base64, &out_base64_len));
        dbg_str(DBG_DETAIL, "base64 encode:%s", out_base64);

        EXEC(base64_decode(out_base64, strlen(out_base64), out_base64, &out_base64_len));
        dbg_buf(DBG_DETAIL, "base64 decode:", out_base64, out_base64_len);
        
        EXEC(sk->decrypt(sk, out, out_len, out, &out_len));
        dbg_str(DBG_DETAIL, "ecb_aes decode:%s", out);
        dbg_buf(DBG_DETAIL, "ecb_aes decode:", out, out_len);
        THROW_IF(memcmp(in, out, out_len) != 0, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_skcipher_ecb_aes, in:%s, out:%s", in, out);
    } FINALLY {
        object_destroy(sk);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_skcipher_ecb_aes);