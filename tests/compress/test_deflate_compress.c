#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/compress/Compress.h>

static int test_deflate_compress_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int fd, ret, out_len = 0;
    allocator_t *allocator = allocator_get_default_instance();
    Compress *c;
    File *in_file, *out_file;
    char buf[65536];
    char *src_file = "./tests/res/test_deflate_uncompress.txt";
    char *dst_file = "./tests/res/test_deflate_compress.txt";
    char expect_out[25] = {0xcb, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0x28, 0xcf, 0x2f, 0xca, 
                           0x49, 0xd1, 0x51, 0xc8, 0x40, 0x70, 0x8c, 0x50, 0x78, 0x03,
                           0x27, 0x05, 0x00};

    TRY {
        dbg_str(DBG_SUC, "test_deflate_compress_file start ...");
        in_file = object_new(allocator, "File", NULL);
        out_file = object_new(allocator, "File", NULL);
        in_file->open(in_file, src_file, "r+");
        out_file->open(out_file, dst_file, "w+");

        c = object_new(allocator, "DeflateCompress", NULL);
        THROW_IF(c == NULL, -1);

        EXEC(c->compress(c, in_file, 200, out_file, &out_len));
        out_file->seek(out_file, 0, SEEK_SET);
        out_file->read(out_file, (uint8_t *)buf, out_len);
        dbg_str(DBG_VIP, "out_len:%d", out_len);
        dbg_buf(DBG_VIP, "expect out:", expect_out, sizeof(expect_out));
        dbg_buf(DBG_VIP, "compressed:", buf, sizeof(expect_out));
        THROW_IF(strcmp(expect_out, buf) != 0, -1);

    } CATCH (ret) { } FINALLY {
        object_destroy(c);
        object_destroy(in_file);
        object_destroy(out_file);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_deflate_compress_file);

static int test_deflate_uncompress_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int fd, ret, out_len = 0;
    allocator_t *allocator = allocator_get_default_instance();
    File *in_file, *out_file;
    Compress *c;
    char buf[65536];
    char *src_file = "./tests/res/test_deflate_compress.txt";
    char *dst_file = "./tests/res/test_deflate_uncompress.txt";
    char expect_plaintext[512] = "hello world, hello world2, hello world, hello world2, hello world, "
                                 "hello world2, hello world, hello world2, hello world, hello world2, "
                                 "hello world, hello world2";
    
    TRY {
        dbg_str(DBG_SUC, "test_deflate_uncompress_file start ...");
        in_file = object_new(allocator, "File", NULL);
        out_file = object_new(allocator, "File", NULL);
        in_file->open(in_file, src_file, "r+");
        out_file->open(out_file, dst_file, "w+");

        c = object_new(allocator, "DeflateCompress", NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->uncompress(c, in_file, 23, out_file, &out_len));

        out_file->seek(out_file, 0, SEEK_SET);
        out_file->read(out_file, (uint8_t *)buf, out_len);
        dbg_str(DBG_VIP, "out_len:%d", out_len);
        dbg_str(DBG_VIP, "expect plaintext:%s", expect_plaintext);
        dbg_str(DBG_VIP, "uncompressed plaintext:%s", buf);
        THROW_IF(strcmp(expect_plaintext, buf) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(c);
        object_destroy(in_file);
        object_destroy(out_file);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_deflate_uncompress_file);

static int test_deflate_uncompress_buf(TEST_ENTRY *entry)
{
    Compress *c;
    allocator_t *allocator = allocator_get_default_instance();
    char plaintext[512] = "hello world, hello world2, hello world, hello world2, hello world, "
                          "hello world2, hello world, hello world2, hello world, hello world2, "
                          "hello world, hello world2";
    char compress_out[25] = {0xcb, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0x28, 0xcf, 0x2f, 0xca, 
                             0x49, 0xd1, 0x51, 0xc8, 0x40, 0x70, 0x8c, 0x50, 0x78, 0x03,
                             0x27, 0x05, 0x00};
    char uncompress_out[1024] = {0};
    unsigned long compress_out_len = sizeof(compress_out);
    unsigned long uncompress_out_len = sizeof(uncompress_out);
    int ret;

    TRY {
        c = object_new(allocator, "DeflateCompress", NULL);
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
REGISTER_TEST_FUNC(test_deflate_uncompress_buf);
