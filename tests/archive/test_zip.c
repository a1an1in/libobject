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
	char *zip_file = "./tests/res/test_zip.zip";
    char *file = "test_extract.txt";

    TRY {
        dbg_str(DBG_SUC, "test extract zip");

        archive = object_new(allocator, "Zip", NULL);
		archive->open(archive, zip_file, "r+");
		archive->extract_file(archive, file);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_extract_deflate_file);

static int test_zip_extract_store_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
	char *zip_file = "./tests/res/test_zip.zip";
    char *file = "subdir/test_extract2.txt";

    TRY {
        dbg_str(DBG_SUC, "test extract zip");

        archive = object_new(allocator, "Zip", NULL);
		archive->open(archive, zip_file, "r+");
		archive->extract_file(archive, file);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_extract_store_file);

static int test_zip_add_file(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char *file = "./tests/res/test_zip.txt";
    char *tar_name = "./tests/res/test_zip.zip";

    TRY {
        dbg_str(DBG_SUC, "test add files to zip");

        archive = object_new(allocator, "Zip", NULL);
		archive->open(archive, tar_name, "w+");

		archive->add_file(archive, file);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip_add_file);