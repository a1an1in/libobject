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
        dbg_str(DBG_SUC, "test_tar_list");
        fs_mkdir("./tests/output/tar/", 0777);
        archive = object_new(allocator, "Tar", NULL);
        archive->set_extracting_path(archive, "./tests/output/tar/");
		archive->open(archive, zip_file, "r+");
		archive->list(archive, &infos);
        SET_CATCH_INT_PARS(infos->count(infos), 0);
        THROW_IF(infos->count(infos) != 2, -1);
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
    int ret, i, count;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    archive_file_info_t *info;
    Vector *infos;
	char *archive_file = "./tests/res/tar/test_extract.tar";
    char *ref_file = "./tests/res/tar/test_extract.txt";

    TRY {
        dbg_str(DBG_SUC, "test_tar_extract_file");
        fs_mkdir("./tests/output/tar/", 0777);
        archive = object_new(allocator, "Tar", NULL);
		EXEC(archive->open(archive, archive_file, "r+"));
		EXEC(archive->set_extracting_path(archive, "./tests/output/tar/"));

        EXEC(archive->list(archive, &infos));
        THROW_IF((count = infos->count(infos)) == 0, -1);
        for (i = 0; i < count; i++) {
            EXEC(infos->peek_at(infos, i, &info));
            if(strcmp(info->file_name, "test_extract.txt") != 0) continue;
            EXEC(archive->extract_file(archive, info));
            ret = assert_file_equal("./tests/output/tar/test_extract.txt", ref_file);
            THROW_IF(ret == 1, 1);
        }
		THROW(-1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/tar/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_tar_extract_file);

static int test_tar_extract_all(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
	char *src_file = "./tests/res/tar/test_extract.tar";
    char *ref_file = "./tests/res/tar/test_extract.txt";

    TRY {
        dbg_str(DBG_SUC, "test_tar_extract_all");

        fs_mkdir("./tests/output/tar/", 0777);
        archive = object_new(allocator, "Tar", NULL);
		EXEC(archive->open(archive, src_file, "r+"));
        EXEC(archive->set_extracting_path(archive, "./tests/output/tar/"));
		EXEC(archive->extract(archive));
        ret = assert_file_equal("./tests/output/tar/test_extract.txt", ref_file);
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/tar/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_tar_extract_all);

static int test_tar_add_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char *file1 = "test_gzip.txt";
    char *file2 = "subdir/test.txt";
    char *tar_name = "./tests/output/tar/test_create.tar";

    TRY {
        dbg_str(DBG_SUC, "test_tar_add_file");

        fs_mkdir("./tests/output/tar/", 0777);
        archive = object_new(allocator, "Tar", NULL);
		EXEC(archive->open(archive, tar_name, "w+"));
        EXEC(archive->set_extracting_path(archive, "./tests/output/tar/"));
        EXEC(archive->set_adding_path(archive, "./tests/res/tar/"));
		EXEC(archive->add_file(archive, file1));
        EXEC(archive->add_file(archive, file2));
        EXEC(archive->save(archive));

        EXEC(archive->extract(archive));
        ret = assert_file_equal("./tests/output/tar/test_gzip.txt", "./tests/res/tar/test_gzip.txt");
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/tar/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_tar_add_file);

static int test_tar_add_all(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char *tar_name = "./tests/output/tar/test_add.tar";

    TRY {
        dbg_str(DBG_SUC, "test_tar_add_all");
        fs_mkdir("./tests/output/tar", 0777);
        archive = object_new(allocator, "Tar", NULL);
		EXEC(archive->open(archive, tar_name, "w+"));
        EXEC(archive->set_adding_path(archive, "./tests/res/tar"));

        EXEC(archive->add(archive));
        EXEC(archive->save(archive));
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        // fs_rmdir("./tests/output/tar/");
    }

    return ret;
}
REGISTER_TEST_CMD(test_tar_add_all);