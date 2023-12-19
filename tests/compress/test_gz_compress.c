#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/compress/Compress.h>

static int test_gz_compress_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int fd, ret;
    allocator_t *allocator = allocator_get_default_instance();
    Compress *c;
    char *src_file = "./tests/res/gz/test_gzip.txt";
    char *dst_file = "./tests/output/gz/test_gzip.gz";
    char *reference_file = "./tests/res/gz/test_gzip.gz";

    TRY {
        dbg_str(DBG_SUC, "test_gzip start ...");

        fs_mkdir("./tests/output/gz", 0777);
        c = object_new(allocator, "GZCompress", NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->compress_file(c, src_file, dst_file));
        ret = assert_file_equal(dst_file, reference_file);
        THROW_IF(ret != 1, -1);

    } CATCH (ret) { } FINALLY {
        object_destroy(c);
        fs_rmdir("./tests/output/gz/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_gz_compress_file);

static int test_gz_uncompress_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int fd, ret;
    allocator_t *allocator = allocator_get_default_instance();
    Compress *c;
    char buf[65536];
    char *src_file = "./tests/res/gz/test_gzip.gz";
    char *dst_file = "./tests/output/gz/test_gzip.txt";
    char *reference_file = "./tests/res/gz/test_gzip.txt";

    TRY {
        dbg_str(DBG_SUC, "test_gzip start ...");

        fs_mkdir("./tests/output/gz", 0777);
        c = object_new(allocator, "GZCompress", NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->uncompress_file(c, src_file, dst_file));
        ret = assert_file_equal(dst_file, reference_file);
        THROW_IF(ret != 1, -1);

    } CATCH (ret) { } FINALLY {
        object_destroy(c);
        fs_rmdir("./tests/output/gz/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_gz_uncompress_file);