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

void *dl_get_dynamic_lib_base_address(pid_t pid, const char *module_name)
{
    FILE *fp;
    void* addr = NULL;
    char *pch;
    char filename[32];
    char line[1024];
    int ret;

    TRY {
        if (pid < 0) {
            /* self process */
            snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
        } else {
            snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
        }

        fp = fopen(filename, "r");
        THROW_IF(fp == NULL, -1);

        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                pch = strtok(line, "-");
                addr = strtoul(pch, NULL, 16);

                if (addr == 0x8000)
                    addr = 0;
                break;
            }
        }
    } CATCH (ret) { addr = NULL; } FINALLY {
        fclose(fp);
    }
    
    return (void *)addr;
}

void* dl_get_remote_function_adress(pid_t target_pid, const char* module_name, void* local_addr)
{
	void* local_handle, *remote_handle;
    int ret;

    TRY {
        local_handle = dl_get_dynamic_lib_base_address(-1, module_name);
        THROW_IF(local_handle == NULL, -1);
	    remote_handle = dl_get_dynamic_lib_base_address(target_pid, module_name);
        THROW_IF(remote_handle == NULL, -1);
        return (void *)((uint64_t)local_addr + (uint64_t)remote_handle - (uint64_t)local_handle);
    } CATCH (ret) {}
	
    return NULL;
}

int dl_get_dynamic_lib_path(pid_t pid, const char *module_name, char *path, int len)
{
    FILE *fp;
    char *p1, *p2;
    char filename[32];
    char line[1024] = {0};
    int ret;

    TRY {
        if (pid < 0) {
            /* self process */
            snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
        } else {
            snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
        }

        fp = fopen(filename, "r");
        THROW_IF(fp == NULL, -1);

        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                printf("line:%s", line);
                p1 = strchr(line, '/');
                p2 = strstr(p1, ".so");
                p2[3] = '\0';
                memcpy(path, p1, strlen(p1));
                break;
            }
        }
    } CATCH (ret) {} FINALLY {
        fclose(fp);
    }
    
    return ret;
}
#endif

