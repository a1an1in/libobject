/**
 * @file Digest.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-04-25
 */

#include <libobject/crypto/Digest_Sha1.h>
#include "sha1.h"

static int __construct(Digest_Sha1 *digest, char *init_str)
{
    allocator_t *allocator = digest->parent.parent.allocator;
    digest_sha1_t *ctx;

    ctx = (digest_sha1_t *)allocator_mem_alloc(allocator, sizeof(digest_sha1_t));
    if (ctx != NULL) {
        digest->context = ctx;
    } else {
        dbg_str(DBG_ERROR, "Digest_Sha1 alloc error");
    }

    return 1;
}

static int __deconstruct(Digest_Sha1 *digest)
{
    allocator_t *allocator = digest->parent.parent.allocator;

    if (digest->context) {
        allocator_mem_free(allocator, digest->context);
    }
    return 0;
}

static int __init(Digest_Sha1 *digest)
{
    dbg_str(DBG_SUC, "Digest_Sha1 init");
    return digest_sha1_init((digest_sha1_t *)digest->context);
}

static int __update(Digest_Sha1 *digest, const void *data, unsigned int size)
{
    return digest_sha1_update((digest_sha1_t *)digest->context, data, size);
}

static int __final(Digest_Sha1 *digest, uint8_t *result, unsigned int size)
{
    uint8_t tmp[20];

    if (size < 20) return -1;
    digest_sha1_final((digest_sha1_t *)digest->context, tmp);
    memcpy(result, tmp, 20);
    return 1;
}

static class_info_entry_t digest_class_info[] = {
    Init_Obj___Entry(0, Digest, parent),
    Init_Nfunc_Entry(1, Digest_Sha1, construct, __construct),
    Init_Nfunc_Entry(2, Digest_Sha1, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Digest_Sha1, get, NULL),
    Init_Vfunc_Entry(4, Digest_Sha1, set, NULL),
    Init_Vfunc_Entry(5, Digest_Sha1, init, __init),
    Init_Vfunc_Entry(6, Digest_Sha1, update, __update),
    Init_Vfunc_Entry(7, Digest_Sha1, final, __final),
    Init_End___Entry(8, Digest_Sha1),
};
REGISTER_CLASS("Digest_Sha1", digest_class_info);

static int
test_obj_sha1(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    char *expect = "f7c3bc1d808e04732adf679965ccc34ca7ae3441";
    char *test = "123456789";
    unsigned char result[20] = {0};
    char result_hex[100] = {0};
    int i, ret;
    Digest *digest;

    dbg_str(DBG_SUC, "Digest_Sha1 in");
    digest = object_new(allocator, "Digest_Sha1", NULL);
    dbg_str(DBG_SUC, "run at here");
    digest->init(digest);
    dbg_str(DBG_SUC, "run at here");
    digest->update(digest, test, strlen(test));
    dbg_str(DBG_SUC, "run at here");
    digest->final(digest, result, 20);

    for (i= 0; i < 20; i++) {
        sprintf(result_hex + strlen(result_hex), "%02x", result[i]);
    }

    ret = assert_equal(result_hex, expect, strlen(expect));
    if (ret == 1) {
        dbg_str(DBG_SUC, "sucess"); 
    } else {
        dbg_str(DBG_ERROR, "expect:%s result:%s", expect, result_hex);
    }
    object_destroy(digest);

    return ret;
}
REGISTER_TEST_FUNC(test_obj_sha1);
