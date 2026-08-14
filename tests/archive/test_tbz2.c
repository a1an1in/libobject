#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/archive/Archive.h>

/*
 * Tbz2 (tar.bz2 wrapper) roundtrip: compress a file to .bz2 then uncompress
 * it back and compare.
 *
 * The source is a checked-in resource file, so no temp input needs to be
 * created.  Note: the paths returned via `char **file_out` alias internal
 * storage of the Tbz2 object and get clobbered by the next compress/uncompress
 * call, so the test immediately copies them with strdup() and frees only
 * those copies.
 */
static int test_tbz2_roundtrip(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    char *src = "./tests/archive/res/test.txt";
    char *bz2 = NULL, *bz2_owned = NULL;
    char *out = NULL, *out_owned = NULL;

    TRY {
        fs_mkdir("./tests/archive/output/bz2", 0777);

        archive = object_new(allocator, "Tbz2", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->set_extracting_path(archive, "./tests/archive/output/bz2/"));
        EXEC(archive->compress(archive, src, &bz2));
        bz2_owned = strdup(bz2);   /* detach from the object's internal buffer */
        THROW_IF(bz2_owned == NULL || fs_is_exist(bz2_owned) != 1, -1);

        EXEC(archive->uncompress(archive, bz2_owned, &out));
        out_owned = strdup(out);   /* detach from the object's internal buffer */
        THROW_IF(out_owned == NULL || fs_is_exist(out_owned) != 1, -1);
        ret = assert_file_equal(src, out_owned);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        if (bz2_owned) { fs_rmfile(bz2_owned); free(bz2_owned); }
        if (out_owned) { fs_rmfile(out_owned); free(out_owned); }
        fs_rmdir("./tests/archive/output/bz2/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_tbz2_roundtrip);

/* interoperability: a .bz2 produced by Tbz2/Bz2Compress must be accepted by
 * the system `bzip2` tool (skipped when bzip2 is not installed, exit 127) */
static int test_tbz2_our_compress_bzip2_decompress(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    char *src = "./tests/archive/res/test.txt";
    char *bz2 = NULL, *bz2_owned = NULL;
    char *out = "./tests/archive/output/bz2/interop_out.txt";
    char cmd[1024];
    int status;

    TRY {
        fs_mkdir("./tests/archive/output/bz2", 0777);

        archive = object_new(allocator, "Tbz2", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->set_extracting_path(archive, "./tests/archive/output/bz2/"));
        EXEC(archive->compress(archive, src, &bz2));
        bz2_owned = strdup(bz2);   /* detach from the object's internal buffer */
        THROW_IF(bz2_owned == NULL || fs_is_exist(bz2_owned) != 1, -1);

        /* decompress with the system tool and compare */
        snprintf(cmd, sizeof(cmd), "bzip2 -dc %s > %s 2>/dev/null", bz2_owned, out);
        status = system(cmd);
        if (status != -1 && status != (127 << 8)) {
            THROW_IF(status != 0, -1);
            THROW_IF(fs_is_exist(out) != 1, -1);
            ret = assert_file_equal(src, out);
            THROW_IF(ret != 1, -1);
        }
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        if (bz2_owned) { fs_rmfile(bz2_owned); free(bz2_owned); }
        fs_rmfile(out);
        fs_rmdir("./tests/archive/output/bz2/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_tbz2_our_compress_bzip2_decompress);

/* reverse interoperability: a .bz2 produced by the system `bzip2` tool must
 * be decompressible by our Tbz2/Bz2Compress implementation (skipped when
 * bzip2 is not installed, exit 127) */
static int test_tbz2_bzip2_compress_our_decompress(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    char *src = "./tests/archive/res/test.txt";
    char *bz2 = "./tests/archive/output/bz2/tool_created.bz2";
    char *out = NULL, *out_owned = NULL;
    char cmd[1024];
    int status;

    TRY {
        fs_mkdir("./tests/archive/output/bz2", 0777);

        /* the system tool compresses the file */
        snprintf(cmd, sizeof(cmd), "bzip2 -c %s > %s 2>/dev/null", src, bz2);
        status = system(cmd);
        if (status != -1 && status != (127 << 8)) {
            THROW_IF(status != 0, -1);
            THROW_IF(fs_is_exist(bz2) != 1, -1);

            /* our implementation decompresses it and must match the original */
            archive = object_new(allocator, "Tbz2", NULL);
            THROW_IF(archive == NULL, -1);
            EXEC(archive->set_extracting_path(archive, "./tests/archive/output/bz2/"));
            EXEC(archive->uncompress(archive, bz2, &out));
            out_owned = strdup(out);   /* detach from the object's internal buffer */
            THROW_IF(out_owned == NULL || fs_is_exist(out_owned) != 1, -1);
            ret = assert_file_equal(src, out_owned);
            THROW_IF(ret != 1, -1);
        }
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        if (out_owned) { fs_rmfile(out_owned); free(out_owned); }
        fs_rmfile(bz2);
        fs_rmdir("./tests/archive/output/bz2/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_tbz2_bzip2_compress_our_decompress);
