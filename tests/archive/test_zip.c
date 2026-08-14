#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <fcntl.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/archive/Archive.h>

/*
 * zip compression method values (mirror src/archive/zip/Zip.h, which is not
 * part of the public include tree).
 */
#define ZIP_METHOD_STORED   0
#define ZIP_METHOD_DEFLATED 8

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
    char *test_str = "./tests/archive/output/zip/subdir/test_zip_extract2.txt";
    char *value;
    int ret, count;

    TRY {
        dbg_str(DBG_INFO, "test_zip_regex");
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
    char *zip_file = "./tests/archive/res/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_INFO, "test extract zip");
        archive = object_new(allocator, "Zip", NULL);
        archive->set_extracting_path(archive, "./tests/archive/output/zip/");
        archive->open(archive, zip_file, "r+");
        archive->list(archive, &infos);
        SET_CATCH_INT_PARS(infos->count(infos), 0);
        THROW_IF(infos->count(infos) != 2, -1);
    } CATCH (ret) { 
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/archive/output/zip/");
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
	char *zip_file = "./tests/archive/res/zip/test_zip.zip";
    char *file = "test_zip_extract.txt";
    char *reference_file1 = "./tests/archive/res/zip/test_zip_extract.txt";
    char *reference_file2 = "./tests/archive/res/zip/test_zip_extract2.txt";

    TRY {
        dbg_str(DBG_INFO, "test extract zip");
        // zip extract file only need file_name,  so we don't have to search 
        // archive_file_info
        info.file_name = file;

        archive = object_new(allocator, "Zip", NULL);
        archive->set_extracting_path(archive, "./tests/archive/output/zip/");
        archive->open(archive, zip_file, "r+");
        archive->extract_file(archive, &info);
        ret = assert_file_equal("./tests/archive/output/zip/test_zip_extract.txt", reference_file1);
        THROW_IF(ret != 1, -1);
        ret = assert_file_equal("./tests/archive/output/zip/test_zip_extract.txt", reference_file2);
        THROW_IF(ret != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/archive/output/zip/");
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
	char *zip_file = "./tests/archive/res/zip/test_zip.zip";
    char *file = "subdir/test_zip_extract2.txt";
    char *reference_file1 = "./tests/archive/res/zip/test_zip_extract2.txt";
    char *reference_file2 = "./tests/archive/res/zip/test_zip_extract.txt";

    TRY {
        dbg_str(DBG_INFO, "test extract zip");
        info.file_name = file;

        archive = object_new(allocator, "Zip", NULL);
        archive->set_extracting_path(archive, "./tests/archive/output/zip/");
        archive->open(archive, zip_file, "r+");
        archive->extract_file(archive, &info);
        ret = assert_file_equal("./tests/archive/output/zip/subdir/test_zip_extract2.txt", reference_file1);
        THROW_IF(ret != 1, -1);
        ret = assert_file_equal("./tests/archive/output/zip/subdir/test_zip_extract2.txt", reference_file2);
        THROW_IF(ret != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/archive/output/zip/");
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
    char *ref_file1 = "./tests/archive/res/zip/test_zip_extract.txt";
    char *ref_file2 = "./tests/archive/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/archive/res/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_INFO, "test add files to zip");

        fs_mkdir("./tests/archive/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        archive->open(archive, tar_name, "r");
        archive->set_extracting_path(archive, "./tests/archive/output/zip/");
        archive->list(archive, &infos);
        archive->extract_files(archive, infos);
        ret = assert_file_equal("./tests/archive/output/zip/test_zip_extract.txt", ref_file1);
        THROW_IF(ret != 1, -1);
        ret = assert_file_equal("./tests/archive/output/zip/subdir/test_zip_extract2.txt", ref_file2);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/archive/output/zip/");
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
    char *ref_file1 = "./tests/archive/res/zip/test_zip_extract.txt";
    char *ref_file2 = "./tests/archive/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/archive/res/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_INFO, "test add files to zip");

        fs_mkdir("./tests/archive/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        archive->open(archive, tar_name, "r");
        archive->set_extracting_path(archive, "./tests/archive/output/zip/");
        EXEC(archive->extract(archive));
        ret = assert_file_equal("./tests/archive/output/zip/test_zip_extract.txt", ref_file1);
        THROW_IF(ret != 1, -1);
        ret = assert_file_equal("./tests/archive/output/zip/subdir/test_zip_extract2.txt", ref_file2);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/archive/output/zip/");
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
    char *ref_file1 = "./tests/archive/res/zip/test_zip_extract.txt";
    char *ref_file2 = "./tests/archive/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/archive/res/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_INFO, "test add files to zip");

        fs_mkdir("./tests/archive/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->set_extracting_path(archive, "./tests/archive/output/zip/"));
        EXEC(archive->set_wildchard(archive, SET_INCLUSIVE_WILDCHARD_TYPE, "subdir.*"));
        EXEC(archive->extract(archive));

        THROW_IF(fs_is_exist("./tests/archive/output/zip/test_zip_extract.txt"), -1);
        ret = assert_file_equal("./tests/archive/output/zip/subdir/test_zip_extract2.txt", ref_file2);
        THROW_IF(ret != 1, -1);

    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/archive/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_extract_with_regex);

/*
 * ---------------------------------------------------------------------------
 * add (create) tests.  These used to be manual-only commands; they are now
 * part of the automatic suite and each one verifies the created archive by
 * reopening it and listing it.
 * ---------------------------------------------------------------------------
 */

/* deterministic pseudo-random file; used to exercise the multi-chunk
 * deflate path (> 16KB) and to make the archive bigger than 512 bytes */
static int write_random_file(const char *path, size_t size)
{
    FILE *f = fopen(path, "wb");
    uint32_t seed = 0x12345678u;
    size_t i;

    if (f == NULL) return -1;
    for (i = 0; i < size; i++) {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;
        fputc((int)(seed & 0xff), f);
    }
    fclose(f);

    return 0;
}

static int test_zip_add_1_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    Vector *infos;
    char *file = "./tests/archive/res/zip/test_zip_deflate.txt";
    char *tar_name = "./tests/archive/output/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_INFO, "test add files to zip");
        fs_mkdir("./tests/archive/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        archive->open(archive, tar_name, "w+");

        memset(&info, 0, sizeof(info));
        info.file_name = file;
        archive->add_file(archive, &info);
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        /* reopen and verify the entry is there */
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(tar_name);
        fs_rmdir("./tests/archive/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_add_1_file);

static int test_zip_add_2_files(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    Vector *infos;
    char *file1 = "./tests/archive/res/zip/test_zip_deflate.txt";
    char *file2 = "./tests/archive/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/archive/output/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_INFO, "test add files to zip");
        fs_mkdir("./tests/archive/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "w+"));

        memset(&info, 0, sizeof(info));
        info.file_name = file1;
        EXEC(archive->add_file(archive, &info));
        memset(&info, 0, sizeof(info));
        info.file_name = file2;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        /* reopen and verify both entries are there */
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 2, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(tar_name);
        fs_rmdir("./tests/archive/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_add_2_files);

static int test_zip_add_files(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info = {0};
    Vector *infos;
    char *file1 = "./tests/archive/res/zip/test_zip_deflate.txt";
    char *file2 = "./tests/archive/res/zip/test_zip_extract2.txt";
    char *tar_name = "./tests/archive/output/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_INFO, "test add files to zip");
        fs_mkdir("./tests/archive/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        archive->open(archive, tar_name, "w+");

        memset(&info, 0, sizeof(info));
        info.file_name = file1;
        archive->add_adding_file_info(archive, &info);
        memset(&info, 0, sizeof(archive_file_info_t));
        info.file_name = file2;
        archive->add_adding_file_info(archive, &info);

        EXEC(archive->add_files(archive, archive->adding_file_infos));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        /* reopen and verify both entries are there */
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 2, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(tar_name);
        fs_rmdir("./tests/archive/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_add_files);

static int test_zip_add_all(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    Vector *infos;
    char *tar_name = "./tests/archive/output/zip/test_zip.zip";

    TRY {
        dbg_str(DBG_INFO, "test add files to zip");
        fs_mkdir("./tests/archive/output/zip", 0777);
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "w+"));
        EXEC(archive->set_adding_path(archive, "./tests/archive/res/zip"));

        EXEC(archive->add(archive));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        /* reopen and verify at least one entry was stored */
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) < 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(tar_name);
        fs_rmdir("./tests/archive/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_add_all);

/*
 * A 128KB pseudo-random file:
 *   - the produced archive is much larger than 512 bytes, exercising the
 *     end-of-central-directory tail search (which previously read the head);
 *   - the deflate payload exceeds 16KB, exercising the multi-chunk deflate
 *     (which previously dropped everything after the first 16KB).
 */
static int test_zip_add_roundtrip_deflate(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    Vector *infos;
    char *src = "./test_zip_roundtrip.bin";
    char *tar_name = "./tests/archive/output/zip/test_zip_roundtrip.zip";
    char *out = "./tests/archive/output/zip/./test_zip_roundtrip.bin";

    TRY {
        THROW_IF(write_random_file(src, 128 * 1024) != 0, -1);
        fs_mkdir("./tests/archive/output/zip", 0777);

        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "w+"));
        memset(&info, 0, sizeof(info));
        info.file_name = src;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        /* reopen: this also verifies the tail EOCD search on a large archive */
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->set_extracting_path(archive, "./tests/archive/output/zip/"));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 1, -1);

        memset(&info, 0, sizeof(info));
        info.file_name = src;
        EXEC(archive->extract_file(archive, &info));
        ret = assert_file_equal(src, out);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(src);
        fs_rmfile(tar_name);
        fs_rmfile(out);
        fs_rmdir("./tests/archive/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_add_roundtrip_deflate);

/* store (no compression) roundtrip: entry must be stored, and extract back
 * must reproduce the original bytes exactly */
static int test_zip_add_roundtrip_stored(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info, *info_ptr;
    Vector *infos;
    char *src = "./test_zip_stored_src.txt";
    char *tar_name = "./tests/archive/output/zip/test_zip_stored.zip";
    char *out = "./tests/archive/output/zip/./test_zip_stored_src.txt";

    TRY {
        {
            FILE *f = fopen(src, "wb");
            THROW_IF(f == NULL, -1);
            fwrite("stored payload, stored payload, stored payload\n", 1, 46, f);
            fclose(f);
        }
        fs_mkdir("./tests/archive/output/zip", 0777);

        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "w+"));
        memset(&info, 0, sizeof(info));
        info.file_name = src;
        info.compression_method = ZIP_METHOD_STORED;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        /* reopen: the entry must be reported as stored */
        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->set_extracting_path(archive, "./tests/archive/output/zip/"));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 1, -1);
        info_ptr = NULL;
        EXEC(infos->peek_at(infos, 0, &info_ptr));
        THROW_IF(info_ptr == NULL, -1);
        THROW_IF(info_ptr->compression_method != ZIP_METHOD_STORED, -1);

        memset(&info, 0, sizeof(info));
        info.file_name = src;
        EXEC(archive->extract_file(archive, &info));
        ret = assert_file_equal(src, out);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(src);
        fs_rmfile(tar_name);
        fs_rmfile(out);
        fs_rmdir("./tests/archive/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_add_roundtrip_stored);

/* an empty file must survive a roundtrip (deflated) */
static int test_zip_add_roundtrip_empty(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    Vector *infos;
    char *src = "./test_zip_empty_src.txt";
    char *tar_name = "./tests/archive/output/zip/test_zip_empty.zip";
    char *out = "./tests/archive/output/zip/./test_zip_empty_src.txt";

    TRY {
        {
            FILE *f = fopen(src, "wb");
            THROW_IF(f == NULL, -1);
            fclose(f);
        }
        fs_mkdir("./tests/archive/output/zip", 0777);

        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "w+"));
        memset(&info, 0, sizeof(info));
        info.file_name = src;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->set_extracting_path(archive, "./tests/archive/output/zip/"));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 1, -1);

        memset(&info, 0, sizeof(info));
        info.file_name = src;
        EXEC(archive->extract_file(archive, &info));
        /* the extracted file must exist and be empty */
        THROW_IF(fs_get_size(out) != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(src);
        fs_rmfile(tar_name);
        fs_rmfile(out);
        fs_rmdir("./tests/archive/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_add_roundtrip_empty);

/* validate a produced archive against the external `unzip` tool when it is
 * installed (skipped otherwise, since it is an optional dependency) */
static int test_zip_validate_with_unzip(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    char *src1 = "./test_zip_validate_1.bin";
    char *src2 = "./test_zip_validate_2.txt";
    char *tar_name = "./tests/archive/output/zip/test_zip_validate.zip";
    char cmd[1024];
    int status;

    TRY {
        THROW_IF(write_random_file(src1, 32 * 1024) != 0, -1);
        {
            FILE *f = fopen(src2, "wb");
            THROW_IF(f == NULL, -1);
            fwrite("plain stored data\n", 1, 18, f);
            fclose(f);
        }
        fs_mkdir("./tests/archive/output/zip", 0777);

        archive = object_new(allocator, "Zip", NULL);
        EXEC(archive->open(archive, tar_name, "w+"));
        memset(&info, 0, sizeof(info));
        info.file_name = src1;
        EXEC(archive->add_file(archive, &info));
        memset(&info, 0, sizeof(info));
        info.file_name = src2;
        info.compression_method = ZIP_METHOD_STORED;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        /* run `unzip -t`; if unzip is missing the shell reports 127 and we
         * treat the check as skipped */
        snprintf(cmd, sizeof(cmd), "unzip -t %s > /dev/null 2>&1", tar_name);
        status = system(cmd);
        if (status != -1 && status != (127 << 8)) {
            THROW_IF(status != 0, -1);
        }
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(src1);
        fs_rmfile(src2);
        fs_rmfile(tar_name);
        fs_rmdir("./tests/archive/output/zip/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_zip_validate_with_unzip);
