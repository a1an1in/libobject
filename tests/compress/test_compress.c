#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/compress/Compress.h>

static int test_zcompress_buf(TEST_ENTRY *entry)
{
    Compress *c;
    allocator_t *allocator = allocator_get_default_instance();
    char plaintext[100] = "hello world";
    char compress_out[1024] = {0};
    char uncompress_out[1024] = {0};
    int compress_out_len = sizeof(compress_out);
    int uncompress_out_len = sizeof(uncompress_out);
    int ret;

    TRY {
        c = object_new(allocator, "ZCompress", NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->deflate_buf(c, plaintext, strlen(plaintext), compress_out, &compress_out_len));
        EXEC(c->inflate_buf(c, compress_out, compress_out_len, uncompress_out, &uncompress_out_len));
        THROW_IF(strlen(plaintext) != uncompress_out_len, -1);
        THROW_IF(memcmp(plaintext, uncompress_out, strlen(plaintext)) != 0, -1);
    } CATCH(ret) {
    } FINALLY {
        object_destroy(c);
    };

    return ret;
}
REGISTER_TEST_FUNC(test_zcompress_buf);


