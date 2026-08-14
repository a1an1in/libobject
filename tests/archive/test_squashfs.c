#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/archive/Archive.h>

/* add -> save -> reopen -> list -> extract -> compare.
 * SquashFS 目录项只存裸文件名(如 test.txt), 不支持含 '/' 的路径,
 * 所以镜像内名字为 basename, 解压到 extracting_path 下同名文件 */
static int test_squashfs_add_and_extract_one_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    Vector *infos;
    char *src = "./tests/archive/res/test.txt";
    char *img = "./tests/archive/output/squashfs/test.sqfs";
    char *out = "./tests/archive/output/squashfs/test.txt";

    TRY {
        fs_mkdir("./tests/archive/output/squashfs", 0777);

        archive = object_new(allocator, "Squashfs", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->open(archive, img, "w+"));
        memset(&info, 0, sizeof(info));
        info.file_name = src;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        archive = object_new(allocator, "Squashfs", NULL);
        EXEC(archive->open(archive, img, "r"));
        EXEC(archive->set_extracting_path(archive, "./tests/archive/output/squashfs/"));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 1, -1);

        memset(&info, 0, sizeof(info));
        info.file_name = src;               /* 按 basename 匹配 */
        EXEC(archive->extract_file(archive, &info));
        ret = assert_file_equal(src, out);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(img);
        fs_rmfile(out);
        fs_rmdir("./tests/archive/output/squashfs/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_squashfs_add_and_extract_one_file);

/* two files roundtrip */
static int test_squashfs_add_and_extract_two_files(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    Vector *infos;
    char *src1 = "./tests/archive/res/test.txt";
    char *src2 = "./tests/archive/res/tar/test_extract.txt";
    char *img = "./tests/archive/output/squashfs/two.sqfs";
    char *out1 = "./tests/archive/output/squashfs/test.txt";
    char *out2 = "./tests/archive/output/squashfs/test_extract.txt";

    TRY {
        fs_mkdir("./tests/archive/output/squashfs", 0777);

        archive = object_new(allocator, "Squashfs", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->open(archive, img, "w+"));
        memset(&info, 0, sizeof(info));
        info.file_name = src1;
        EXEC(archive->add_file(archive, &info));
        memset(&info, 0, sizeof(info));
        info.file_name = src2;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        archive = object_new(allocator, "Squashfs", NULL);
        EXEC(archive->open(archive, img, "r"));
        EXEC(archive->set_extracting_path(archive, "./tests/archive/output/squashfs/"));
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
        fs_rmfile(img);
        fs_rmfile(out1);
        fs_rmfile(out2);
        fs_rmdir("./tests/archive/output/squashfs/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_squashfs_add_and_extract_two_files);

/* 互操作: 我们用 Squashfs 生成的镜像, 必须能被标准工具 `unsquashfs` 解压
 * (工具未安装时跳过, exit 127), 且解压出的文件内容与源一致 */
static int test_squashfs_our_image_unsquashfs_extract(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret = 1;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    char *src = "./tests/archive/res/test.txt";
    char *img = "./tests/archive/output/squashfs/interop.sqfs";
    char *dir = "./tests/archive/output/squashfs/interop_dir";
    char cmd[1024];
    int status;

    TRY {
        fs_mkdir("./tests/archive/output/squashfs", 0777);

        archive = object_new(allocator, "Squashfs", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->open(archive, img, "w+"));
        memset(&info, 0, sizeof(info));
        info.file_name = src;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));
        object_destroy(archive);
        archive = NULL;

        snprintf(cmd, sizeof(cmd), "unsquashfs -d %s %s > /dev/null 2>&1", dir, img);
        status = system(cmd);
        if (status != -1 && status != (127 << 8)) {
            THROW_IF(status != 0, -1);
        }
        /* 工具可用且解压成功时, 校验内容与源一致; 否则视为跳过 */
        if (status == 0) {
            char out_file[1024];
            snprintf(out_file, sizeof(out_file), "%s/test.txt", dir);
            ret = assert_file_equal(src, out_file);
            THROW_IF(ret != 1, -1);
        }
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(img);
        system("rm -rf ./tests/archive/output/squashfs/interop_dir");
        fs_rmdir("./tests/archive/output/squashfs/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_squashfs_our_image_unsquashfs_extract);

/* 互操作(反向): 标准工具 `mksquashfs` 生成的镜像, 必须能被我们的 Squashfs
 * 读取并解压出内容一致的 test.txt (工具未安装时跳过, exit 127) */
static int test_squashfs_mksquashfs_image_our_extract(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive = NULL;
    archive_file_info_t info;
    Vector *infos;
    char *src = "./tests/archive/res/test.txt";
    char *img = "./tests/archive/output/squashfs/mksq.sqfs";
    char *out_dir = "./tests/archive/output/squashfs/mksq/";
    char *out = "./tests/archive/output/squashfs/mksq/test.txt";
    char cmd[1024];
    int status;

    TRY {
        fs_mkdir("./tests/archive/output/squashfs", 0777);
        fs_mkdir(out_dir, 0777);

        /* 用 mksquashfs 生成镜像(关闭 fragment/xattr/export, 与我们的写入端一致) */
        snprintf(cmd, sizeof(cmd), "mksquashfs %s %s -noappend -no-fragments -no-xattrs -no-exports > /dev/null 2>&1",
                src, img);
        status = system(cmd);
        if (status != -1 && status != (127 << 8)) {
            THROW_IF(status != 0, -1);
        }
        if (status != 0) THROW(1);   /* 工具不可用, 跳过 */

        archive = object_new(allocator, "Squashfs", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->open(archive, img, "r"));
        EXEC(archive->set_extracting_path(archive, out_dir));
        EXEC(archive->list(archive, &infos));
        THROW_IF(infos->count(infos) != 1, -1);

        memset(&info, 0, sizeof(info));
        info.file_name = "test.txt";
        EXEC(archive->extract_file(archive, &info));
        ret = assert_file_equal(src, out);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmfile(img);
        fs_rmfile(out);
        fs_rmdir(out_dir);
        fs_rmdir("./tests/archive/output/squashfs/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_squashfs_mksquashfs_image_our_extract);
