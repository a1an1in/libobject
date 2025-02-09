#if (!defined(WINDOWS_USER_MODE))
#define _GNU_SOURCE

#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>
#include <libobject/core/utils/string.h>
#include <libobject/attacher/dynamic_lib.h>

void * dl_get_func_addr_by_name(char *name, char *lib_name)
{
    void *handle = NULL;
    void *addr = NULL;

    /* open the needed object */
    handle = dlopen(lib_name, RTLD_LOCAL | RTLD_LAZY);
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
        dbg_str(DBG_DETAIL, "local_addr:%p,  remote_handle:%p, local_handle:%p", local_addr, (uint64_t)remote_handle, (uint64_t)local_handle);
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
                // printf("line:%s", line);
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

int dl_get_dynamic_name(pid_t pid, void *func_addr, char *module_name, int len)
{
    FILE *fp;
    char *p1, *p2, *temp = NULL, *p;
    char filename[32];
    char line[1024] = {0};
    long *addr[2];
    char *addr_str[32] = {0};
    int i = 0, ret;

    TRY {
        if (pid < 0) {
            /* self process */
            snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
        } else {
            snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
        }
        sprintf(addr_str, "%lx", (long unsigned int)func_addr);

        fp = fopen(filename, "r");
        THROW_IF(fp == NULL, -1);

        while (fgets(line, sizeof(line), fp)) {
            p = line;
            for (p = strtok_r(p, "- ", &temp), i = 0; i < 2 && p != NULL;
                p = strtok_r(NULL, "- ", &temp), i++) {
                *(addr + i) = str_hex_to_integer(p);
                // printf("tok:%s, i:%d, *(addr + i):%lx\n", p, i, *(addr + i));
            }

            if (func_addr < (void *)addr[0]  || func_addr > (void *)addr[1]) continue;

            p1 = strrchr(temp, '/');
            p2 = strstr(p1, ".so");
            p2[3] = '\0';
            THROW_IF(len < strlen(p1), -1);
            memcpy(module_name, p1 + 1, strlen(p1 + 1));
            break;
        }   
    } CATCH (ret) {} FINALLY {
        fclose(fp);
    }
    
    return ret;
}

int dl_parse_dynamic_table(pid_t pid, Interval_Tree *tree)
{
    FILE *fp;
    char *p1, *p2, *temp = NULL, *p;
    char filename[32];
    char line[1024] = {0};
    long *addr[2];
    int i = 0, ret;
    allocator_t *allocator;
    char *module_name;
    interval_tree_node_t *node;

    TRY {
        if (pid < 0) {
            /* self process */
            snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
        } else {
            snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
        }
        allocator = tree->obj.allocator;

        fp = fopen(filename, "r");
        THROW_IF(fp == NULL, -1);

        while (fgets(line, sizeof(line), fp)) {
            p = line;
            for (p = strtok_r(p, "- ", &temp), i = 0; i < 2 && p != NULL;
                p = strtok_r(NULL, "- ", &temp), i++) {
                *(addr + i) = str_hex_to_integer(p);
                // printf("tok:%s, i:%d, *(addr + i):%lx\n", p, i, *(addr + i));
            }

            p1 = strrchr(temp, '/');
            if (p1 == NULL) continue;
            p2 = strstr(p1, ".so");
            if (p2 == NULL) continue;
            p2[3] = '\0';
            module_name = allocator_mem_zalloc(allocator, strlen(p1 + 1) + 1);
            memcpy(module_name, p1 + 1, strlen(p1 + 1));
            dbg_str(DBG_INFO, "dl_parse_dynamic_table, addr1:%p, addr2:%p, moudle name:%s", addr[0], addr[1], module_name);
            
            node = allocator_mem_zalloc(allocator, sizeof(interval_tree_node_t));
            node->value = module_name;
            node->start = addr[0];
            node->end = addr[1];
            tree->add(tree, node->start, node);
        }   
    } CATCH (ret) {} FINALLY {
        fclose(fp);
    }
    
    return ret;
}

int dl_get_dynamic_lib_name_from_interval_tree(Interval_Tree *tree, void *func_addr, char *module_name, int len)
{
    int ret;
    interval_tree_node_t *t = NULL;

    TRY {
        EXEC(tree->search(tree, func_addr, (void **)&t));
        THROW_IF(len < strlen(t->value) || t == NULL, -1);
        memcpy(module_name, t->value, strlen(t->value));
        dbg_str(DBG_INFO, "get_dynamic_lib_name, start:%p, end:%p, module_name:%s", t->start, t->end, module_name);
    } CATCH (ret) {} FINALLY {}

    return ret;
}

#endif

