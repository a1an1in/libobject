/**
 * @file CipherAlgo.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-07-25
 */

#include <libobject/crypto/CipherAlgo.h>

typedef struct cipher_algo_pad_policy_s {
    int (*policy)(CipherAlgo *algo, int offset);
} cipher_algo_pad_policy_t;

static int __cipher_algo_pad_null_policy(CipherAlgo *algo, int offset)
{
    dbg_str(DBG_DETAIL, "run at here");
    return 0;
}

static int __cipher_algo_pad_zero_policy(CipherAlgo *algo, int offset)
{
    char *last_block = algo->last_block;
    int i, block_size, ret;

    TRY {
        EXEC(algo->get_block_size(algo, &block_size));
            
        for (i = 0; i < block_size - offset; i++) {
            last_block[offset + i] = 0;
        }
        dbg_buf(DBG_DETAIL, "pad_zero, last block:", last_block, block_size);
    } CATCH (ret) {
    }

    return ret;
}

static int __cipher_algo_pad_pkcs7_policy(CipherAlgo *algo, int offset)
{
    char *last_block = algo->last_block;
    int i, block_size, ret;

    TRY {
        EXEC(algo->get_block_size(algo, &block_size));
            
        for (i = 0; i < block_size - offset; i++) {
            last_block[offset + i] = block_size - offset;
        }
        dbg_buf(DBG_DETAIL, "pad_pkcs7, last block:", last_block, block_size);
    } CATCH (ret) {
    }

    return ret;
}

cipher_algo_pad_policy_t g_cipher_algo_pad_policy[ENTRY_TYPE_MAX_TYPE] = {
    [SKCIPHER_PADDING_NO]    = {.policy = __cipher_algo_pad_null_policy},
    [SKCIPHER_PADDING_ZERO]  = {.policy = __cipher_algo_pad_zero_policy},
    [SKCIPHER_PADDING_PKCS7] = {.policy = __cipher_algo_pad_pkcs7_policy},
};

static int __cipher_algo_unpad_null_policy(CipherAlgo *algo, u8 *out, u32 *out_len)
{
    dbg_str(DBG_DETAIL, "run at here");
    return 0;
}

static int __cipher_algo_unpad_zero_policy(CipherAlgo *algo, u8 *out, u32 *out_len)
{
    char *last_block = algo->last_block;
    int i, block_size, ret, count;

    TRY {
        EXEC(algo->get_block_size(algo, &block_size));
        THROW_IF(out_len == NULL && out == NULL, -1);
        THROW_IF(out[*out_len] != 0, -1); // zero padding, at least the last byte is zero

        for (i = *out_len; i >= *out_len - block_size && out[i] == 0; i--) ;
        
        *out_len = (i + 1);
    } CATCH (ret) {
    }

    return ret;
}

static int __cipher_algo_unpad_pkcs7_policy(CipherAlgo *algo, u8 *out, u32 *out_len)
{
    char *last_block = algo->last_block;
    int i, block_size, ret, count;

    TRY {
        EXEC(algo->get_block_size(algo, &block_size));
        THROW_IF(out_len == NULL && out == NULL, -1);

        count = out[*out_len - 1];
        THROW_IF(count > 16 || count <= 0, -1);

        for (i = *out_len; i > *out_len - count; i--) {
            out[i] = 0;
        }
        
        *out_len = *out_len - count;
    } CATCH (ret) {
    }

    return ret;
}

typedef struct cipher_algo_unpad_policy_s {
    int (*policy)(CipherAlgo *algo, u8 *out, u32 *out_len);
} cipher_algo_unpad_policy_t;

cipher_algo_unpad_policy_t g_cipher_algo_unpad_policy[ENTRY_TYPE_MAX_TYPE] = {
    [SKCIPHER_PADDING_NO]    = {.policy = __cipher_algo_unpad_null_policy},
    [SKCIPHER_PADDING_ZERO]  = {.policy = __cipher_algo_unpad_zero_policy},
    [SKCIPHER_PADDING_PKCS7] = {.policy = __cipher_algo_unpad_pkcs7_policy},
};

static int __construct(CipherAlgo *algo)
{
    algo->padding = SKCIPHER_PADDING_PKCS7;

    return 0;
}

static int __deconstruct(CipherAlgo *algo)
{
    object_destroy(algo->sub_algo);

    return 0;
}

static int __pad(CipherAlgo *algo, const u8 *in, const u32 in_len)
{
    int (*policy)(CipherAlgo *algo, int offset);
    int block_size, ret;

    TRY {
        EXEC(algo->get_block_size(algo, &block_size));
        THROW_IF(algo->padding > SKCIPHER_PADDING_MAX, -1);
        THROW_IF(in_len > block_size, -1);
        
        if (in_len) {
            memcpy(algo->last_block, in, in_len);
        }
        policy = g_cipher_algo_pad_policy[algo->padding].policy;
        THROW_IF(policy == NULL, -1);

        EXEC(policy(algo, in_len));
    } CATCH (ret) {
    }
    
    return ret;
}

static int __unpad(CipherAlgo *algo, u8 *out, u32 *out_len)
{
    int (*policy)(CipherAlgo *algo, u8 *out, u32 *out_len);
    int block_size, ret;

    TRY {
        EXEC(algo->get_block_size(algo, &block_size));
        THROW_IF(algo->padding > SKCIPHER_PADDING_MAX, -1);

        policy = g_cipher_algo_unpad_policy[algo->padding].policy;
        THROW_IF(policy == NULL, -1);

        EXEC(policy(algo, out, out_len));
    } CATCH (ret) {
    }
    
    return ret;
}

static int __set_iv(CipherAlgo *algo, void *iv)
{
    int block_size, ret;
    
    TRY {
        EXEC(algo->get_block_size(algo, &block_size));

        memcpy(algo->iv, iv, block_size);
    } CATCH (ret) {
    }
    
    return ret;
}

static class_info_entry_t cipher_algo_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , CipherAlgo, construct, __construct),
    Init_Nfunc_Entry(2 , CipherAlgo, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , CipherAlgo, set_key, NULL),
    Init_Vfunc_Entry(4 , CipherAlgo, encrypt, NULL),
    Init_Vfunc_Entry(5 , CipherAlgo, decrypt, NULL),
    Init_Vfunc_Entry(6 , CipherAlgo, pad, __pad),
    Init_Vfunc_Entry(7 , CipherAlgo, unpad, __unpad),
    Init_Vfunc_Entry(8 , CipherAlgo, get_block_size, NULL),
    Init_Vfunc_Entry(9 , CipherAlgo, set_iv, __set_iv),
    Init_End___Entry(10, CipherAlgo),
};
REGISTER_CLASS("CipherAlgo", cipher_algo_class_info);

static int
test_aes(TEST_ENTRY *entry, void *argc, void *argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_alloc();
    CipherAlgo *algo;
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
		0xcc, 0xdd, 0xee, 0xff};
	
	uint8_t out[16] = {0}; // 128

    dbg_str(DBG_SUC, "test_aes in");
    algo = object_new(allocator, "Aes", NULL);
    algo->set_key(algo, key, sizeof(key));

    algo->encrypt(algo, in, sizeof(in), out, sizeof(out));

    for (int i = 0; i < 16; i++) {
        printf("%x ", out[i]);
    }
    printf("\n");
    algo->decrypt(algo, out, sizeof(out), out, sizeof(out));
    for (int i = 0; i < 16; i++) {
        printf("%x ", out[i]);
    }
    printf("\n");

    object_destroy(algo);

    return ret;
}
REGISTER_TEST_FUNC(test_aes);