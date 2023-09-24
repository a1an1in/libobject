#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/archive/Archive.h>

static int test_gzip(TEST_ENTRY *entry, int argc, void **argv)
{
    int fd, ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;
    char buf[65536];
    char *src_file = "/home/alan/workspace/libobject/res/test.txt.gz";
    char *dst_file = "/home/alan/workspace/libobject/res/test.txt";

    TRY {
        dbg_str(DBG_SUC, "test_gzip start ...");

        archive = object_new(allocator, "Gzip", NULL);
        THROW_IF(archive == NULL, -1);
        EXEC(archive->uncompress(archive, src_file, dst_file));

    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_gzip);


