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
        dbg_str(DBG_DETAIL, "run at here, str:%x", str);
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

static int __encrypt(Skcipher *sk, const u8 *in, const u32 in_len, u8 *out, u32 out_len)
{
    CipherAlgo *algo;

    algo = sk->algo;
    TRY_EXEC(algo->encrypt(algo, in, in_len, out, out_len));
   
    return 0;
}

static int __decrypt(Skcipher *sk, const u8 *in, const u32 in_len, u8 *out, u32 out_len)
{
    CipherAlgo *algo;

    algo = sk->algo;
    TRY_EXEC(algo->decrypt(algo, in, in_len, out, out_len));
   
    return 0;
}

static int __set_key(Skcipher *sk, char *in_key, unsigned int key_len)
{
    CipherAlgo *algo;

    algo = sk->algo;
    TRY_EXEC(algo->set_key(algo, in_key, key_len));
   
    return 0;
}

static class_info_entry_t sk_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Skcipher, construct, __construct),
    Init_Nfunc_Entry(2, Skcipher, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Skcipher, set_algo, __set_algo),
    Init_Vfunc_Entry(4, Skcipher, set_key, __set_key),
    Init_Vfunc_Entry(5, Skcipher, encrypt, __encrypt),
    Init_Vfunc_Entry(6, Skcipher, decrypt, __decrypt),
    Init_End___Entry(7, Skcipher),
};
REGISTER_CLASS("Skcipher", sk_class_info);

static int
test_skcipher_ecb_aes(TEST_ENTRY *entry, void *argc, void *argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_alloc();
    Skcipher *sk;
	uint8_t key[] = {
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13,
		0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1a, 0x1b,
		0x1c, 0x1d, 0x1e, 0x1f};
	uint8_t in[] = {
		0x00, 0x11, 0x22, 0x33,
		0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb,
		0xcc, 0xdd, 0xee, 0xff,
        0x00, 0x11, 0x22, 0x33,
		0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb,
		0xcc, 0xdd, 0xee, 0xff};
	uint8_t out[32] = {0}; // 128

    TRY {
        dbg_str(DBG_SUC, "test_skciphter_ecb_aes in");
        sk = object_new(allocator, "Skcipher", NULL);
        dbg_str(DBG_DETAIL, "run at here, sk:%x", sk);

        sk->set_algo(sk, "Ecb(Aes)");
        sk->set_key(sk, key, sizeof(key));
        sk->encrypt(sk, in, sizeof(in), out, sizeof(out));

        for (int i = 0; i < sizeof(in); i++) {
            printf("%x ", out[i]);
        }
        printf("\n");
        sk->decrypt(sk, out, sizeof(out), out, sizeof(out));
        for (int i = 0; i < sizeof(in); i++) {
            printf("%x ", out[i]);
        }
        printf("\n");

        object_destroy(sk);
    } CATCH (ret) {
    }

    return ret;
}
REGISTER_TEST_FUNC(test_skcipher_ecb_aes);