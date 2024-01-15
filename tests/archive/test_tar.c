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

static int test_tar_list(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    Vector *infos;
	char *zip_file = "./tests/res/tar/test_extract.tar";

    TRY {
        dbg_str(DBG_SUC, "test extract tar");
        fs_mkdir("./tests/output/tar/", 0777);
        archive = object_new(allocator, "Tar", NULL);
        archive->set_extracting_path(archive, "./tests/output/tar/");
		archive->open(archive, zip_file, "r+");
		archive->list(archive, &infos);
        SET_CATCH_INT_PARS(infos->count(infos), 0);
        THROW_IF(infos->count(infos) != 1, -1);
    } CATCH (ret) { 
        CATCH_SHOW_INT_PARS(DBG_ERROR);
    } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/tar/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_tar_list);

static int test_tar_extract_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    archive_file_info_t info;
	char *src_file = "./tests/res/tar/test_extract.tar";
    char *dst_file = "./tests/res/tar/test_extract.txt";

    TRY {
        dbg_str(DBG_SUC, "test_tar");

        info.file_name = "test_extract.txt";
        info.offset = 0;

        fs_mkdir("./tests/output/tar/", 0777);
        archive = object_new(allocator, "Tar", NULL);
		archive->open(archive, src_file, "r+");
		archive->set_extracting_path(archive, "./tests/output/tar/");
		archive->extract_file(archive, &info);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/tar/");
    }

    return ret;
}
REGISTER_TEST_CMD(test_tar_extract_file);

static int test_tar_extract_all(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
	char *src_file = "./tests/res/tar/test_extract.tar";
    char *dst_file = "./tests/res/tar/test_extract.txt";

    TRY {
        dbg_str(DBG_SUC, "test_tar");

        fs_mkdir("./tests/output/tar/", 0777);
        archive = object_new(allocator, "Tar", NULL);
		archive->open(archive, src_file, "r+");
        archive->set_extracting_path(archive, "./tests/output/tar/");
		archive->extract(archive);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        // fs_rmdir("./tests/output/tar/");
    }

    return ret;
}
REGISTER_TEST_CMD(test_tar_extract_all);

static int test_tar_add_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char *file1 = "./tests/res/tar/test_gzip.txt";
    char *file2 = "./tests/res/tar/subdir/test.txt";
    char *tar_name = "./tests/res/tar/test_create.tar";

    TRY {
        dbg_str(DBG_SUC, "test_tar");

        fs_mkdir("./tests/output/tar/", 0777);
        archive = object_new(allocator, "Tar", NULL);
		archive->open(archive, tar_name, "w+");
        archive->set_extracting_path(archive, "./tests/output/tar/");

		archive->add_file(archive, file1);
        archive->add_file(archive, file2);

        archive->save(archive);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/tar/");
    }

    return ret;
}
REGISTER_TEST_CMD(test_tar_add_file);