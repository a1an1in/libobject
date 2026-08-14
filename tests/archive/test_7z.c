#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/archive/Archive.h>

/* add -> save -> reopen -> list -> extract -> compare, using a checked-in
 * resource file as the source (the entry name is the full source path, so the
 * extracted file lands under the extracting path with the same relative path) */
static int test_7z_add_and_extract_one_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    Vector *infos;
    char *src = "./tests/archive/res/test.txt";
    char *tar_name = "./tests/archive/output/7z/test.7z";
    char *out = "./tests/archive/output/7z/./tests/archive/res/test.txt";

    TRY {
        fs_mkdir("./tests/archive/output/7z", 0777);

        archive = object_new(allocator, "SevenZip", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->open(archive, tar_name, "w+"));
        memset(&info, 0, sizeof(info));
        info.file_name = src;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        archive = object_new(allocator, "SevenZip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->set_extracting_path(archive, "./tests/archive/output/7z/"));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 1, -1);

        memset(&info, 0, sizeof(info));
        info.file_name = src;
        EXEC(archive->extract_file(archive, &info));
        ret = assert_file_equal(src, out);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(tar_name);
        fs_rmfile(out);
        fs_rmdir("./tests/archive/output/7z/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_7z_add_and_extract_one_file);

/* add two files -> save -> reopen -> list (2 entries) -> extract both */
static int test_7z_add_and_extract_two_files(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    Vector *infos;
    char *src1 = "./tests/archive/res/test.txt";
    char *src2 = "./tests/archive/res/tar/test_extract.txt";
    char *tar_name = "./tests/archive/output/7z/two.7z";
    char *out1 = "./tests/archive/output/7z/./tests/archive/res/test.txt";
    char *out2 = "./tests/archive/output/7z/./tests/archive/res/tar/test_extract.txt";

    TRY {
        fs_mkdir("./tests/archive/output/7z", 0777);

        archive = object_new(allocator, "SevenZip", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->open(archive, tar_name, "w+"));
        memset(&info, 0, sizeof(info));
        info.file_name = src1;
        EXEC(archive->add_file(archive, &info));
        memset(&info, 0, sizeof(info));
        info.file_name = src2;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        archive = object_new(allocator, "SevenZip", NULL);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->set_extracting_path(archive, "./tests/archive/output/7z/"));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 2, -1);

        memset(&info, 0, sizeof(info));
        info.file_name = src1;
        EXEC(archive->extract_file(archive, &info));
        memset(&info, 0, sizeof(info));
        info.file_name = src2;
        EXEC(archive->extract_file(archive, &info));
        ret = assert_file_equal(src1, out1);
        THROW_IF(ret != 1, -1);
        ret = assert_file_equal(src2, out2);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(tar_name);
        fs_rmfile(out1);
        fs_rmfile(out2);
        fs_rmdir("./tests/archive/output/7z/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_7z_add_and_extract_two_files);

/* 互操作: 我们用 SevenZip 打包的 7z, 必须能被标准工具 `7z`/`7za` 校验打开
 * (工具未安装时跳过, exit 127) */
static int test_7z_our_archive_other_tool_extract(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    char *src = "./tests/archive/res/test.txt";
    char *tar_name = "./tests/archive/output/7z/interop.7z";
    char cmd[1024];
    int status;

    TRY {
        fs_mkdir("./tests/archive/output/7z", 0777);

        archive = object_new(allocator, "SevenZip", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->open(archive, tar_name, "w+"));
        memset(&info, 0, sizeof(info));
        info.file_name = src;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        /* 标准工具校验打开我们的 7z; 工具不可用(127)视为跳过 */
        snprintf(cmd, sizeof(cmd), "7za t %s > /dev/null 2>&1 || 7z t %s > /dev/null 2>&1", tar_name, tar_name);
        status = system(cmd);
        if (status != -1 && status != (127 << 8)) {
            THROW_IF(status != 0, -1);
        }
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(tar_name);
        fs_rmdir("./tests/archive/output/7z/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_7z_our_archive_other_tool_extract);

/* 互操作(反向): 标准工具 `7z`/`7za` 打包的 7z, 必须能被我们的 SevenZip
 * 读取并解压出内容一致的 test.txt (工具未安装时跳过, exit 127) */
static int test_7z_other_tool_archive_our_extract(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    archive_file_info_t *info_ptr = NULL;
    Vector *infos;
    char *src = "./tests/archive/res/test.txt";
    char *tar_name = "./tests/archive/output/7z/std.7z";
    char *out_dir = "./tests/archive/output/7z/std/";
    char out[1024];
    char cmd[1024];
    int status;

    TRY {
        fs_mkdir("./tests/archive/output/7z", 0777);
        fs_mkdir(out_dir, 0777);

        /* 用标准 7z 工具打包 */
        snprintf(cmd, sizeof(cmd),
                 "7za a -t7z %s %s > /dev/null 2>&1 || 7z a -t7z %s %s > /dev/null 2>&1",
                 tar_name, src, tar_name, src);
        status = system(cmd);
        if (status != -1 && status != (127 << 8)) {
            THROW_IF(status != 0, -1);
        }
        if (status != 0) THROW(1);   /* 工具不可用, 跳过 */

        archive = object_new(allocator, "SevenZip", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->open(archive, tar_name, "r"));
        EXEC(archive->set_extracting_path(archive, out_dir));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 1, -1);
        EXEC(infos->peek_at(infos, 0, &info_ptr));
        THROW_IF(info_ptr == NULL, -1);

        /* 按标准工具存储的条目名提取, 再与源比对 */
        memset(&info, 0, sizeof(info));
        info.file_name = info_ptr->file_name;
        EXEC(archive->extract_file(archive, &info));
        snprintf(out, sizeof(out), "%s%s", out_dir, info_ptr->file_name);
        ret = assert_file_equal(src, out);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(tar_name);
        system("rm -rf ./tests/archive/output/7z/std");
        fs_rmdir("./tests/archive/output/7z/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_7z_other_tool_archive_our_extract);
