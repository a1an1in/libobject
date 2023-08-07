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

void * dl_get_func_addr_by_name(char *name)
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

int dl_get_func_name_by_addr(void *addr, char *name, unsigned int name_len)
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

#endif

