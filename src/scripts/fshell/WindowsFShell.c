/**
 * @file WindowsFShell.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-10-20
 */

#if (defined(WINDOWS_USER_MODE))
#define _GNU_SOURCE
#include <dlfcn.h>
#include <libobject/core/String.h>
#include "WindowsFShell.h"
#include <libobject/mockery/mockery.h>

static int __construct(WindowsFShell *shell, char *init_str)
{
    return 0;
}

static int __deconstruct(WindowsFShell *shell)
{
    Map *map = shell->parent.map;

    Iterator *cur, *end;
    void *key, *value;

    cur = map->begin(map);
    end = map->end(map);

    for (; !end->equal(end, cur); cur->next(cur)) {
        key = cur->get_kpointer(cur);
        value = cur->get_vpointer(cur);
        dbg_str(DBG_DETAIL, "windows fshell close lib:%s, handle:%p", key, value);
        dlclose(value);
    }

    return 0;
}

static int __load(WindowsFShell *shell, char *lib_name, int flag)
{
    void *handle;
    Map *map = shell->parent.map;
    int ret;
    void *addr;

    TRY {
        handle = dlopen(lib_name, flag);
        THROW_IF(handle == NULL, -1);

        EXEC(map->add(map, lib_name, handle));
    } CATCH (ret) {
        perror("err:");
    }

    return ret;
}

static int __unload(WindowsFShell *shell, char *lib_name, int flag)
{
    void *handle = NULL;
    Map *map = shell->parent.map;
    int ret;

    TRY {
        EXEC(map->remove(map, lib_name, (void **)&handle));
        THROW_IF(handle == NULL, -1);
        EXEC(dlclose(handle));
    } CATCH (ret) {
    }

    return ret;
}

/* 
 *window 只能从动态库能用dlopen， 不能读取链接的静态库，即使是生成bin时链接的静态库。
 *所以，如果需要动态调用，则需要把被调用的函数编译成动态库。
 */
static int __get_func_addr(WindowsFShell *shell, char *lib_name, char *func_name, void **addr)
{
    void *handle = NULL;
    int ret;

    TRY {
        // THROW(0);
        /* open the needed object */
        handle = dlopen(lib_name, RTLD_LOCAL | RTLD_LAZY);
        THROW_IF(handle == NULL, -1);
        dbg_str(DBG_INFO, "handle=%p", handle);

        /* find the address of function and data objects */
        *addr = dlsym(handle, func_name);
        dbg_str(DBG_INFO, "addr=%p", *addr);
    } CATCH (ret) { } FINALLY {
        if (handle != NULL)
            dlclose(handle);
    }

    return ret;
}

static int __get_func_name(WindowsFShell *shell, char *lib_name, void *addr, char *name, unsigned int name_len)
{
    void *handle = NULL;
    Dl_info dl;
    int ret = 0;

    TRY {
        // THROW(0);
        /* open the needed object */
        handle = dlopen(lib_name, RTLD_LOCAL | RTLD_LAZY);
        THROW_IF(handle == NULL, -1);

        /* find the address of function and data objects */
        ret = dladdr(addr, &dl);
        dlclose(handle);
        THROW_IF(ret == 0, -1);

        strncpy(name, dl.dli_sname, name_len - 1);
    } CATCH (ret) { } FINALLY {
        if (handle != NULL)
            dlclose(handle);
    }

    return ret;
}

static int __open_ui(WindowsFShell *shell)
{
    char *linebuf = NULL, *p;
    size_t linebuf_size = 0;
    allocator_t *allocator;
    String *str;
    char c;
    int ret = 0, i, cnt, len;

    TRY {
        THROW(0);
    } CATCH (ret) { }

    return ret;
}

static class_info_entry_t shell_class_info[] = {
    Init_Obj___Entry(0 , FShell, parent),
    Init_Nfunc_Entry(1 , WindowsFShell, construct, __construct),
    Init_Nfunc_Entry(2 , WindowsFShell, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , WindowsFShell, load, __load),
    Init_Vfunc_Entry(4 , WindowsFShell, unload, __unload),
    Init_Vfunc_Entry(5 , WindowsFShell, get_func_addr, __get_func_addr),
    Init_Vfunc_Entry(6 , WindowsFShell, get_func_name, __get_func_name),
    Init_Vfunc_Entry(7 , WindowsFShell, open_ui, NULL),
    Init_Vfunc_Entry(8 , WindowsFShell, run_func, NULL),
    Init_Vfunc_Entry(9 , WindowsFShell, is_statement, NULL),
    Init_End___Entry(10, WindowsFShell),
};
REGISTER_CLASS("WindowsFShell", shell_class_info);
#endif
