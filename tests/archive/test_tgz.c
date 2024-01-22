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

static int test_tgz_extract(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
	char *src_file = "./tests/res/tgz/test.tar.gz";
    char *tar_file_path;

    TRY {
        dbg_str(DBG_SUC, "test_tgz");

        fs_mkdir("./tests/output/tgz/", 0777);
        archive = object_new(allocator, "Tgz", NULL);
        archive->set_extracting_path(archive, "./tests/output/tgz/");
        archive->uncompress(archive, src_file, &tar_file_path);
		archive->open(archive, tar_file_path, "r+");
		archive->extract(archive);

        ret = assert_file_equal("./tests/output/tgz/test.txt", "./tests/res/tgz/test.txt");
        THROW_IF(ret != 1, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/");
    }

    return ret;
}
REGISTER_TEST_CMD(test_tgz_extract);


/*
static int test_tar_add_tgz(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char *src_file = "./tests/res/tar/test_extract.txt";
	char *dst_file = "./tests/res/tar/test_extract.tar";
    char tgz_file_path[1024] = {0}

    TRY {
        dbg_str(DBG_SUC, "test_tar");

        archive = object_new(allocator, "Tar", NULL);
        archive->set_extracting_path(archive, "./test/res/tar/");
		archive->open(archive, dst_file, "r+");
		archive->add_file(archive, src_file);
        archive->compress(archive, dst_file, tgz_file_path);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_tar_add_tgz);
*/