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
        dbg_str(DBG_INFO, "test_tgz");

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
REGISTER_TEST_FUNC(test_tgz_extract);

static int test_tgz_add(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    archive_file_info_t info;
    char *src_file = "./tests/res/tgz/test.txt";
	char *tar_file = "./tests/output/tgz/test.tar";
    char *tgz_file;
    
    TRY {
        dbg_str(DBG_INFO, "test_tar");

        fs_mkdir("./tests/output/tgz/", 0777);
        archive = object_new(allocator, "Tgz", NULL);
        EXEC(archive->set_extracting_path(archive, "./tests/output/tgz/"));
        EXEC(archive->set_adding_path(archive, "./tests/res/tgz/"));
		EXEC(archive->open(archive, tar_file, "w+"));

        info.file_name = src_file;
        EXEC(archive->add_file(archive, &info));
        EXEC(archive->save(archive));

        EXEC(archive->compress(archive, tar_file, &tgz_file));

        EXEC(archive->extract(archive));
        ret = assert_file_equal("./tests/output/tgz/test.txt", "./tests/res/tgz/test.txt");
        THROW_IF(ret != 1, -1);

    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
        fs_rmdir("./tests/output/");
    }

    return ret;
}
REGISTER_TEST_FUNC(test_tgz_add);
