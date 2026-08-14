#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/compress/Compress.h>

static int test_bz2_compress_buf(TEST_ENTRY *entry, int argc, void **argv)
{
    Compress *c;
    allocator_t *allocator = allocator_get_default_instance();
    char plaintext[512] = "hello bzip2, hello bzip2, hello bzip2, hello bzip2, hello bzip2, "
                          "hello bzip2, hello bzip2, hello bzip2, hello bzip2, hello bzip2";
    char compress_out[4096] = {0};
    char uncompress_out[4096] = {0};
    int compress_out_len = sizeof(compress_out);
    int uncompress_out_len = sizeof(uncompress_out);
    int ret;

    TRY {
        c = object_new(allocator, "Bz2Compress", NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->compress_buf(c, plaintext, strlen(plaintext), compress_out, &compress_out_len));
        THROW_IF(compress_out_len <= 0, -1);
        EXEC(c->uncompress_buf(c, compress_out, compress_out_len, uncompress_out, &uncompress_out_len));
        THROW_IF((int)strlen(plaintext) != uncompress_out_len, -1);
        THROW_IF(memcmp(plaintext, uncompress_out, strlen(plaintext)) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(c);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_bz2_compress_buf);

static int test_bz2_compress_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    Compress *c;
    allocator_t *allocator = allocator_get_default_instance();
    char *src = "./tests/compress/res/test.txt";
    char *dst = "./tests/archive/output/bz2/test_bz2_file.bz2";
    char *back = "./tests/archive/output/bz2/test_bz2_file_back.txt";

    TRY {
        fs_mkdir("./tests/archive/output/bz2", 0777);

        c = object_new(allocator, "Bz2Compress", NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->compress_file(c, src, dst));
        THROW_IF(fs_is_exist(dst) != 1, -1);
        EXEC(c->uncompress_file(c, dst, back));
        THROW_IF(fs_is_exist(back) != 1, -1);
        ret = assert_file_equal(src, back);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(c);
        fs_rmfile(dst);
        fs_rmfile(back);
        fs_rmdir("./tests/archive/output/bz2/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_bz2_compress_file);
