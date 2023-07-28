#if (!defined(WINDOWS_USER_MODE))
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>
#include <libobject/scripts/fshell/Attacher.h>

static int test_attacher(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;

    TRY {
        dbg_str(DBG_SUC, "test_attacher");
        attacher = object_new(allocator, "UnixAttacher", NULL);
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher);

#endif

