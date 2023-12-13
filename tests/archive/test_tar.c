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

static int test_tar_extract(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
	char *src_file = "./tests/res/tar/test_extract.tar";
    char *dst_file = "./tests/res/tar/test_extract.txt";

    TRY {
        dbg_str(DBG_SUC, "test_tar");

        archive = object_new(allocator, "Tar", NULL);
		archive->open(archive, src_file, "r+");
		archive->set_path(archive, "./test/res/tar/");
		archive->extract(archive);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_tar_extract);

static int test_tar_add_files(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char *file1 = "./tests/res/tar/test_gzip.txt";
    char *file2 = "./tests/res/tar/subdir/test.txt";
    char *tar_name = "./tests/res/tar/test_create.tar";

    TRY {
        dbg_str(DBG_SUC, "test_tar");

        archive = object_new(allocator, "Tar", NULL);
		archive->open(archive, tar_name, "w+");
        archive->set_path(archive, "./test/res/tar/");

		archive->add_file(archive, file1);
        archive->add_file(archive, file2);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_tar_add_files);