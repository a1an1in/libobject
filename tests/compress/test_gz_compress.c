#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/compress/Compress.h>

static int test_gz_compress_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int fd, ret;
    allocator_t *allocator = allocator_get_default_instance();
    Compress *c;
    char *src_file = "./tests/compress/res/gz/test_gzip.txt";
    char *dst_file = "./tests/compress/output/gz/test_gzip.gz";
    char *back_file = "./tests/compress/output/gz/test_gzip.txt";

    TRY {
        dbg_str(DBG_INFO, "test_gzip start ...");

        fs_mkdir("./tests/compress/output/gz", 0777);
        c = object_new(allocator, "GZCompress", NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->compress_file(c, src_file, dst_file));
        /*
         * NOTE: gzip 头里的 OS 字节跟 zlib 的编译平台有关(linux 是 0x03,
         * mingw 是 0x0b), 所以不能跟 linux 上生成的参考文件逐字节比较,
         * 这里用"压缩后再解压"来校验内容一致。
         */
        EXEC(c->uncompress_file(c, dst_file, back_file));
        ret = assert_file_equal(back_file, src_file);
        THROW_IF(ret != 1, -1);
        /* 生成的必须是合法的 gzip 流(魔数 1f 8b) */
        {
            File *f = object_new(allocator, "File", NULL);
            unsigned char magic[2] = {0};

            THROW_IF(f == NULL, -1);
            f->open(f, dst_file, "rb");
            f->read(f, magic, sizeof(magic));
            THROW_IF(magic[0] != 0x1f || magic[1] != 0x8b, -1);
            object_destroy(f);
        }

    } CATCH (ret) { } FINALLY {
        object_destroy(c);
        fs_rmfile("./tests/compress/output/gz/test_gzip.gz");
        fs_rmfile("./tests/compress/output/gz/test_gzip.txt");
        fs_rmdir("./tests/compress/output/gz/");
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
    char *src_file = "./tests/compress/res/gz/test_gzip.gz";
    char *dst_file = "./tests/compress/output/gz/test_gzip.txt";
    char *reference_file = "./tests/compress/res/gz/test_gzip.txt";

    TRY {
        dbg_str(DBG_INFO, "test_gzip start ...");

        fs_mkdir("./tests/compress/output/gz", 0777);
        c = object_new(allocator, "GZCompress", NULL);
        THROW_IF(c == NULL, -1);
        EXEC(c->uncompress_file(c, src_file, dst_file));
        ret = assert_file_equal(dst_file, reference_file);
        THROW_IF(ret != 1, -1);

    } CATCH (ret) { } FINALLY {
        object_destroy(c);
        fs_rmdir("./tests/compress/output/gz/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_gz_uncompress_file);