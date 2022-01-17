/**
 * @file Digest.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-04-25
 */

#include <libobject/crypto/Digest_HmacSha1_V1.h>
#include "sha1.h"

static int __construct(Digest_HmacSha1 *digest, char *init_str)
{
    digest->context = HMAC_CTX_new();

    return 1;
}

static int __deconstruct(Digest_HmacSha1 *digest)
{
    if (digest->context) {
        HMAC_CTX_free(digest->context);
    }
    return 0;
}

static int __init(Digest_HmacSha1 *digest)
{
    dbg_str(DBG_SUC, "Digest_HmacSha1 not implement init()");
    return -1;
}

static int __init_with_key(Digest_HmacSha1 *digest, char *key, int len)
{
    return HMAC_Init(digest->context, key, len, EVP_sha1());
}

static int __update(Digest_HmacSha1 *digest, const void *data, unsigned int size)
{
    return HMAC_Update(digest->context, data, size);
}

static int __final(Digest_HmacSha1 *digest, uint8_t *result, unsigned int size)
{
    if (size < 20) {
        return -1;
    }
    return HMAC_Final(digest->context, result, &size);
}

static class_info_entry_t digest_class_info[] = {
    Init_Obj___Entry(0, Digest, parent),
    Init_Nfunc_Entry(1, Digest_HmacSha1, construct, __construct),
    Init_Nfunc_Entry(2, Digest_HmacSha1, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Digest_HmacSha1, get, NULL),
    Init_Vfunc_Entry(4, Digest_HmacSha1, set, NULL),
    Init_Vfunc_Entry(5, Digest_HmacSha1, init, __init),
    Init_Vfunc_Entry(6, Digest_HmacSha1, init_with_key, __init_with_key),
    Init_Vfunc_Entry(7, Digest_HmacSha1, update, __update),
    Init_Vfunc_Entry(8, Digest_HmacSha1, final, __final),
    Init_End___Entry(9, Digest_HmacSha1),
};
REGISTER_CLASS("Openssl::Digest_HmacSha1", digest_class_info);

static int
test_hmac_sha1_v1(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_alloc();
    char *expect = "b632dfc6832db7eea43baa54bd1d18e4af23235e";
    char *test = "1111111111";
    char *key = "12345";
    unsigned char result[20] = {0};
    char result_hex[100] = {0};
    int i, ret;
    Digest *digest;

    dbg_str(DBG_SUC, "Digest_HmacSha1 in");
    digest = object_new(allocator, "Openssl::Digest_HmacSha1", NULL);
    digest->init_with_key(digest, key, strlen(key));
    digest->update(digest, test, strlen(test));
    digest->final(digest, result, 20);

    for (i= 0; i < 20; i++) {
        sprintf(result_hex + strlen(result_hex), "%02x", result[i]);
    }

    ret = assert_equal(result_hex, expect, strlen(expect));
    if (ret == 1) {
        dbg_str(DBG_SUC, "sucess, expect:%s result:%s", expect, result_hex);
    } else {
        dbg_str(DBG_ERROR, "expect:%s result:%s", expect, result_hex);
    }
    object_destroy(digest);

    return ret;
}
REGISTER_TEST_FUNC(test_hmac_sha1_v1);
