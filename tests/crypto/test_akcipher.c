#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/crypto/Akcipher.h>

static int test_akcipher_dsa(TEST_ENTRY *entry, void *argc, void *argv)
{
    Akcipher *cipher;
    allocator_t *allocator = allocator_get_default_alloc();
    char plaintext[100] = "hello world";
    char compress_out[1024] = {0};
    char uncompress_out[1024] = {0};
    int compress_out_len = sizeof(compress_out);
    int uncompress_out_len = sizeof(uncompress_out);
    void *key;
    int ret;

    TRY {
        cipher = object_new(allocator, "AkcipherDsa", NULL);
        THROW_IF(cipher == NULL, -1);
        EXEC(cipher->encrypt(cipher, (akcipher_key_type_e)key, plaintext, strlen(plaintext), compress_out, &compress_out_len));
        EXEC(cipher->decrypt(cipher, (akcipher_key_type_e)key, compress_out, compress_out_len, uncompress_out, &uncompress_out_len));
        THROW_IF(strlen(plaintext) != uncompress_out_len, -1);
        THROW_IF(memcmp(plaintext, uncompress_out, strlen(plaintext)) != 0, -1);
    } CATCH(ret) {
    } FINALLY {
        object_destroy(cipher);
    };

    return ret;
}
REGISTER_TEST_FUNC(test_akcipher_dsa);


