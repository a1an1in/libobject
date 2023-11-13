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

static int test_zip_extract(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
	char *src_file = "./tests/res/test_zip.zip";
    char *dst_file = "./tests/res/test_zip.txt";

    TRY {
        dbg_str(DBG_SUC, "test extract zip");

        archive = object_new(allocator, "Zip", NULL);
		archive->open(archive, src_file);
		archive->set_path(archive, "./test/res/");
		archive->extract(archive);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_extract);

static int test_zip_add_files(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char *file1 = "./tests/res/test_zip.txt";
    char *file2 = "./tests/res/subdir/test.txt";
    char *tar_name = "./tests/res/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");

        archive = object_new(allocator, "Zip", NULL);
		archive->open(archive, tar_name);
        archive->set_path(archive, "./test/res/");

		archive->add_file(archive, file1);
        archive->add_file(archive, file2);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_add_files);