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
    char *src_file = "/home/alan/workspace/libobject/res/test_zip.zip";
    char *dst_file = "/home/alan/workspace/libobject/res/test_zip.txt";

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