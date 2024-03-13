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
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_crc32);

static int test_zip_regex(TEST_ENTRY *entry, int argc, void **argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    String *str;
    char *test_str = "./tests/output/zip/subdir/test_zip_extract2.txt";
    char *value;
    int ret, count;

    TRY {
        dbg_str(DBG_SUC, "test_zip_regex");
        str = object_new(allocator, "String", NULL);
        str->assign(str, test_str);
        count = str->find(str, "subdir(.*)", 0, -1);
        SET_CATCH_INT_PARS(count, 0);
        THROW_IF(count != 1, -1);
        value = str->get_found_cstr(str, 0);
        dbg_str(DBG_INFO, "find cstr:%s", value);
    } CATCH (ret) { 
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(str);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_regex);

static int test_zip_list(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    Vector *infos;
    char *zip_file = "./tests/res/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test extract zip");
        archive = object_new(allocator, "Zip", NULL);
        archive->set_extracting_path(archive, "./tests/output/zip/");
        archive->open(archive, zip_file, "r+");
        archive->list(archive, &infos);
        SET_CATCH_INT_PARS(infos->count(infos), 0);
        THROW_IF(infos->count(infos) != 2, -1);
    } CATCH (ret) { 
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_list);

static int test_zip_extract_deflate_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    archive_file_info_t info;
	char *zip_file = "./tests/res/zip/test_zip.zip";
    char *file = "test_zip_extract.txt";
    char *reference_file1 = "./tests/res/zip/test_zip_extract.txt";
    char *reference_file2 = "./tests/res/zip/test_zip_extract2.txt";

    TRY {
        dbg_str(DBG_SUC, "test extract zip");
        // zip extract file only need file_name,  so we don't have to search 
        // archive_file_info
        info.file_name = file;

        archive = object_new(allocator, "Zip", NULL);
        archive->set_extracting_path(archive, "./tests/output/zip/");
        archive->open(archive, zip_file, "r+");
        archive->extract_file(archive, &info);
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
    archive_file_info_t info;
	char *zip_file = "./tests/res/zip/test_zip.zip";
    char *file = "subdir/test_zip_extract2.txt";
    char *reference_file1 = "./tests/res/zip/test_zip_extract2.txt";
    char *reference_file2 = "./tests/res/zip/test_zip_extract.txt";

    TRY {
        dbg_str(DBG_SUC, "test extract zip");
        info.file_name = file;

        archive = object_new(allocator, "Zip", NULL);
        archive->set_extracting_path(archive, "./tests/output/zip/");
        archive->open(archive, zip_file, "r+");
        archive->extract_file(archive, &info);
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

static int test_zip_extract_files(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    Vector *infos;
    char *ref_file1 = "./tests/res/zip/test_zip_extract.txt";
    char *ref_file2 = "./tests/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/res/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");

        fs_mkdir("./tests/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        archive->open(archive, tar_name, "r");
        archive->set_extracting_path(archive, "./tests/output/zip/");
        archive->list(archive, &infos);
        archive->extract_files(archive, infos);
        ret = assert_file_equal("./tests/output/zip/test_zip_extract.txt", ref_file1);
        THROW_IF(ret != 1, -1);
        ret = assert_file_equal("./tests/output/zip/subdir/test_zip_extract2.txt", ref_file2);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_extract_files);

static int test_zip_extract_all(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    Vector *infos;
    char *ref_file1 = "./tests/res/zip/test_zip_extract.txt";
    char *ref_file2 = "./tests/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/res/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");

        fs_mkdir("./tests/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        archive->open(archive, tar_name, "r");
        archive->set_extracting_path(archive, "./tests/output/zip/");
        EXEC(archive->extract(archive));
        ret = assert_file_equal("./tests/output/zip/test_zip_extract.txt", ref_file1);
        THROW_IF(ret != 1, -1);
        ret = assert_file_equal("./tests/output/zip/subdir/test_zip_extract2.txt", ref_file2);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_extract_all);

static int test_zip_extract_with_regex(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    Vector *infos;
    char *ref_file1 = "./tests/res/zip/test_zip_extract.txt";
    char *ref_file2 = "./tests/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/res/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");

        fs_mkdir("./tests/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->set_extracting_path(archive, "./tests/output/zip/"));
        EXEC(archive->set_wildchard(archive, SET_INCLUSIVE_WILDCHARD_TYPE, "subdir.*"));
        EXEC(archive->extract(archive));

        THROW_IF(fs_is_exist("./tests/output/zip/test_zip_extract.txt"), -1);
        ret = assert_file_equal("./tests/output/zip/subdir/test_zip_extract2.txt", ref_file2);
        THROW_IF(ret != 1, -1);

    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_extract_with_regex);

static int test_zip_add_1_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    archive_file_info_t info;
    char *file = "./tests/res/zip/test_zip_deflate.txt";
    char *tar_name = "./tests/output/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");
        fs_mkdir("./tests/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        archive->open(archive, tar_name, "w+");

        info.file_name = file;
        archive->add_file(archive, &info);
        EXEC(archive->save(archive));
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output");
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_add_1_file);

static int test_zip_add_2_files(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    archive_file_info_t info;
    char *file1 = "./tests/res/zip/test_zip_deflate.txt";
    char *file2 = "./tests/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/output/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");
        fs_mkdir("./tests/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "w+"));

        info.file_name = file1;
        EXEC(archive->add_file(archive, &info));
        info.file_name = file2;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output");
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_add_2_files);

static int test_zip_add_files(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    archive_file_info_t info = {0};
    char *file1 = "./tests/res/zip/test_zip_deflate.txt";
    char *file2 = "./tests/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/output/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");
        fs_mkdir("./tests/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        archive->open(archive, tar_name, "w+");

        info.file_name = file1;
        archive->add_adding_file_info(archive, &info);
        memset(&info, 0, sizeof(archive_file_info_t));
        info.file_name = file2;
        archive->add_adding_file_info(archive, &info);

        EXEC(archive->add_files(archive, archive->adding_file_infos));
        EXEC(archive->save(archive));
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        // fs_rmdir("./tests/output");
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_add_files);

static int test_zip_add_all(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char *tar_name = "./tests/output/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");
        fs_mkdir("./tests/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "w+"));
        EXEC(archive->set_adding_path(archive, "./tests/res/zip"));

        EXEC(archive->add(archive));
        EXEC(archive->save(archive));
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        // fs_rmdir("./tests/output/zip/");
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_add_all);