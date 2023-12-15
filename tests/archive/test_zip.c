#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/archive/Archive.h>

static int test_zip_extract_deflate_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
	char *zip_file = "./tests/res/zip/test_zip.zip";
    char *file = "test_zip_extract.txt";
    char *reference_file1 = "./tests/res/zip/test_zip_extract.txt";
    char *reference_file2 = "./tests/res/zip/test_zip_extract2.txt";

    TRY {
        dbg_str(DBG_SUC, "test extract zip");

        archive = object_new(allocator, "Zip", NULL);
        archive->set_path(archive, "./tests/output/zip/");
		archive->open(archive, zip_file, "r+");
		archive->extract_file(archive, file);
        ret = assert_file_equal("./tests/output/zip/test_zip_extract.txt", reference_file1);
        THROW_IF(ret != 1, -1);
        ret = assert_file_equal("./tests/output/zip/test_zip_extract.txt", reference_file2);
        THROW_IF(ret != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_extract_deflate_file);

static int test_zip_extract_store_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
	char *zip_file = "./tests/res/zip/test_zip.zip";
    char *file = "subdir/test_zip_extract2.txt";
    char *reference_file1 = "./tests/res/zip/test_zip_extract2.txt";
    char *reference_file2 = "./tests/res/zip/test_zip_extract.txt";

    TRY {
        dbg_str(DBG_SUC, "test extract zip");

        archive = object_new(allocator, "Zip", NULL);
        archive->set_path(archive, "./tests/output/zip/");
		archive->open(archive, zip_file, "r+");
		archive->extract_file(archive, file);
        ret = assert_file_equal("./tests/output/zip/subdir/test_zip_extract2.txt", reference_file1);
        THROW_IF(ret != 1, -1);
        ret = assert_file_equal("./tests/output/zip/subdir/test_zip_extract2.txt", reference_file2);
        THROW_IF(ret != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_extract_store_file);

static int test_zip_crc32(TEST_ENTRY *entry, int argc, void **argv)
{
    char expect_plaintext[512] = "hello world, hello world2, hello world, hello world2, hello world, "
                                 "hello world2, hello world, hello world2, hello world, hello world2, "
                                 "hello world, hello world2";
    int ret;
    unsigned int result, expect_result = 0xa8e65b40;

    TRY {
        result = (unsigned int)crc32(0, expect_plaintext, strlen(expect_plaintext));
        SET_CATCH_INT_PARS(result, expect_result);
        THROW_IF(result != expect_result, -1);
    } CATCH (ret) { 
        TRY_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_crc32);

static int test_zip_add_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char *file = "./tests/res/zip/test_zip_deflate.txt";
    char *tar_name = "./tests/output/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");
        fs_mkdir("./tests/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
		archive->open(archive, tar_name, "w+");

		archive->add_file(archive, file);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        // fs_rmdir("./tests/output/zip/");
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_add_file);

static int test_zip_add_files(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char *file1 = "./tests/res/zip/test_zip_deflate.txt";
    char *file2 = "./tests/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/output/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");
        fs_mkdir("./tests/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
		archive->open(archive, tar_name, "w+");

		archive->add_file(archive, file1);
        archive->add_file(archive, file2);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        // fs_rmdir("./tests/output/zip/");
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_add_files);


