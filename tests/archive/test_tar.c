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
	char *src_file = "/home/alan/workspace/libobject/res/test.tar";
    char *dst_file = "./res/test_gzip.txt";

    TRY {
        dbg_str(DBG_SUC, "test_tar");

        archive = object_new(allocator, "Tar", NULL);
		archive->open(archive, src_file);
		archive->set_path(archive, "./res/");
		archive->extract(archive);
    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_tar_extract);