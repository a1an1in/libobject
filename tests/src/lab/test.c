#if (!defined(WINDOWS_USER_MODE))
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>

extern int print_outbound(int a, int b, int c, int d, int e, int f, int *g);

void * get_func_addr_by_name(char *name)
{
    void *handle = NULL;
    void *addr = NULL;

    /* open the needed object */
    handle = dlopen(NULL, RTLD_LOCAL | RTLD_LAZY);
    if (handle == NULL) {
        return NULL;
    }
    dbg_str(DBG_DETAIL, "handle=%p", handle);

    /* find the address of function and data objects */
    addr = dlsym(handle, name);
    dbg_str(DBG_DETAIL, "addr=%p", addr);
    dlclose(handle);

    return addr;
}

int get_func_name_by_addr(void *addr, char *name, unsigned int name_len)
{
    void *handle = NULL;
    Dl_info dl;
    int ret = 0;

    /* open the needed object */
    handle = dlopen(NULL, RTLD_LOCAL | RTLD_LAZY);
    if (handle == NULL) {
        return -1;
    }

    /* find the address of function and data objects */
    ret = dladdr(addr, &dl);
    dlclose(handle);

    if (ret == 0) {
        return -1;
    }

    strncpy(name, dl.dli_sname, name_len - 1);

    return 0;
}

int test_get_func_addr(TEST_ENTRY *entry)
{
    char *func_name = "print_outbound";
    void *expect_addr = print_outbound;
    void *addr;
    int ret;

    TRY {
        addr = get_func_addr_by_name(func_name);
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

