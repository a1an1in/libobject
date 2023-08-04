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
    pid_t pid;

    TRY {
        dbg_str(DBG_SUC, "test_attacher");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);

        pid = atoi(argv[1]);
        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));
        EXEC(attacher->call(attacher, 0, 0, 0));
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher);

#endif

