#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/compress/Compress.h>

static int test_zip_compress_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int fd, ret;
    allocator_t *allocator = allocator_get_default_instance();
    Compress *c;
    char *src_file = "/home/alan/workspace/libobject/res/test_zip.txt";
    char *dst_file = "/home/alan/workspace/libobject/res/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test_zip start ...");

        c = object_new(allocator, "ZipCompress", NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->compress_file(c, src_file, dst_file));

    } CATCH (ret) { } FINALLY {
        object_destroy(c);
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_compress_file);

static int test_zip_uncompress_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int fd, ret;
    allocator_t *allocator = allocator_get_default_instance();
    Compress *c;
    char buf[65536];
    char *src_file = "./tests/res/test_zip.zip";
    char *dst_file = "./tesst/res/test_zip.txt";

    TRY {
        dbg_str(DBG_SUC, "test_unzip start ...");

        c = object_new(allocator, "ZipCompress", NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->uncompress_file(c, src_file, dst_file));

    } CATCH (ret) { } FINALLY {
        object_destroy(c);
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_uncompress_file);

static int test_zip_uncompress_buf(TEST_ENTRY *entry)
{
    Compress *c;
    allocator_t *allocator = allocator_get_default_instance();
    char plaintext[512] = "hello world, hello world2, hello world, hello world2, hello world, hello world2, hello world, hello world2, hello world, hello world2, hello world, hello world2";
    char compress_out[25] = {0xcb, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0x28, 0xcf, 0x2f, 0xca, 
                             0x49, 0xd1, 0x51, 0xc8, 0x40, 0x70, 0x8c, 0x50, 0x78, 0x03,
                             0x27, 0x05, 0x00};
    char uncompress_out[1024] = {0};
    unsigned long compress_out_len = sizeof(compress_out);
    unsigned long uncompress_out_len = sizeof(uncompress_out);
    int ret;

    TRY {
        c = object_new(allocator, "ZipCompress", NULL);
        THROW_IF(c == NULL, -1);
        // EXEC(c->compress_buf(c, plaintext, strlen(plaintext), compress_out, &compress_out_len));
        // dbg_str(DBG_VIP, "in len:%d sizeof(uncompress_out):%d", compress_out_len, uncompress_out_len);
        EXEC(c->uncompress_buf(c, compress_out, 25, uncompress_out, &uncompress_out_len));
        dbg_str(DBG_VIP, "uncompress_out:%s", uncompress_out);
        THROW_IF(memcmp(plaintext, uncompress_out, strlen(plaintext)) != 0, -1);
    } CATCH(ret) { } FINALLY {
        object_destroy(c);
    };

    return ret;
}
REGISTER_TEST_FUNC(test_zip_uncompress_buf);
