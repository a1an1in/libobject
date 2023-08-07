#if (!defined(WINDOWS_USER_MODE))
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>
#include <libobject/scripts/fshell/dynamic_lib.h>

extern int print_outbound(int a, int b, int c, int d, int e, int f, int *g);

int test_get_func_addr(TEST_ENTRY *entry)
{
    char *func_name = "print_outbound";
    void *expect_addr = print_outbound;
    void *addr;
    int ret;

    TRY {
        addr = dl_get_func_addr_by_name(func_name);
        THROW_IF(addr == NULL, -1);
        THROW_IF(addr != expect_addr, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_get_func_addr, addr=%p, %s:%p",
                addr, func_name, expect_addr);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_get_func_addr);

#endif

