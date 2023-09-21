#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/archive/Archive.h>

static int test_zip(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Archive *archive;

    TRY {
        dbg_str(DBG_SUC, "test_attacher_call_from_lib");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);

        archive = object_new(allocator, "Zip", NULL);

    } CATCH (ret) { } FINALLY {
        object_destroy(archive);
    }

    return ret;
}
REGISTER_TEST_CMD(test_zip);


