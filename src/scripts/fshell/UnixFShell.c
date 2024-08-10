/**
 * @file UnixFShell.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-10-20
 */

#if (!defined(WINDOWS_USER_MODE))
#define _GNU_SOURCE
#include <dlfcn.h>
#include <libobject/core/String.h>
#include "UnixFShell.h"
#include <libobject/mockery/mockery.h>

static int __construct(UnixFShell *shell, char *init_str)
{
    return 0;
}

static int __deconstruct(UnixFShell *shell)
{
    Map *map = shell->parent.lib_map;
    Iterator *cur, *end;
    void *key;
    fsh_lib_info_t *info;

    cur = map->begin(map);
    end = map->end(map);

    for (; !end->equal(end, cur); cur->next(cur)) {
        key = cur->get_kpointer(cur);
        info = cur->get_vpointer(cur);
        dbg_str(DBG_VIP, "unix fshell close lib:%s, handle:%p", key, info->handle);
        dlclose(info->handle);
    }

    dbg_str(DBG_VIP, "unixfshell deconstruct out");

    return 0;
}

static int __load(UnixFShell *shell, char *lib_name, int flag)
{
    void *handle;
    Map *map = shell->parent.lib_map;
    fsh_lib_info_t *info;
    int ret;
    void *addr;

    TRY {
        handle = dlopen(lib_name, flag);
        THROW_IF(handle == NULL, -1);
        dbg_str(DBG_VIP, "load lib handler:%p", handle);

        info = allocator_mem_alloc(shell->parent.parent.allocator, sizeof(fsh_lib_info_t));
        strcpy(info->path, lib_name);
        info->handle = handle;

        EXEC(map->add(map, info->path, info));
    } CATCH (ret) {
        perror("err:");
    }

    return ret;
}

static int __unload(UnixFShell *shell, char *lib_name, int flag)
{
    fsh_lib_info_t *info;
    Map *map = shell->parent.lib_map;
    int ret;

    TRY {
        EXEC(map->remove(map, lib_name, (void **)&info));
        THROW_IF(info == NULL, -1);
        EXEC(dlclose(info->handle));
    } CATCH (ret) { }

    return ret;
}

static int __get_func_addr(UnixFShell *shell, char *lib_name, char *func_name, void **addr)
{
    void *handle = NULL;
    int ret;

    TRY {
        /* open the needed object */
        handle = dlopen(lib_name, RTLD_LOCAL | RTLD_LAZY);
        THROW_IF(handle == NULL, -1);
        dbg_str(DBG_DETAIL, "handle=%p", handle);

        /* find the address of function and data objects */
        *addr = dlsym(handle, func_name);
        dbg_str(DBG_DETAIL, "addr=%p", *addr);
    } CATCH (ret) {
    } FINALLY {
        if (handle != NULL)
            dlclose(handle);
    }

    return ret;
}

static int __get_func_name(UnixFShell *shell, char *lib_name, void *addr, char *name, unsigned int name_len)
{
    void *handle = NULL;
    Dl_info dl;
    int ret = 0;

    TRY {
        /* open the needed object */
        handle = dlopen(lib_name, RTLD_LOCAL | RTLD_LAZY);
        THROW_IF(handle == NULL, -1);

        /* find the address of function and data objects */
        ret = dladdr(addr, &dl);
        dlclose(handle);
        THROW_IF(ret == 0, -1);

        strncpy(name, dl.dli_sname, name_len - 1);
    } CATCH (ret) {
    }

    return ret;
}

static int __open_ui(UnixFShell *shell)
{
    char *linebuf = NULL, *p;
    size_t linebuf_size = 0;
    allocator_t *allocator;
    String *str;
    char c;
    int ret = 0, i, cnt, len;
    // char linebuf[1024];

    TRY {
        THROW_IF(shell == NULL, -1);
        allocator = shell->parent.parent.allocator;
        str = object_new(allocator, "String", NULL);
        
        while(shell->parent.close_flag != 1) {
            printf("%s", shell->parent.prompt);
            /*
             * Alternatively, before calling getline(), *lineptr can contain a
             * pointer to a malloc(3)-allocated buffer *n bytes in size.  If the
             * buffer is not large enough to hold the line, getline() resizes it
             * with realloc(3), updating *lineptr and *n as necessary.
             **/

            // c = getchar();
            // printf("%c", c);
            if(getline(&linebuf, &linebuf_size, stdin) < 0)
                break;
            
            if (shell->is_statement(shell, linebuf) == 1) {
                dbg_buf(DBG_DETAIL, "linebuf:", linebuf, strlen(linebuf));
                str->assign(str, linebuf);
                EXEC(shell->run_func(shell, str));
            } else {
                // dbg_buf(DBG_DETAIL, "linebuf:", linebuf, strlen(linebuf));
                continue;
            }
        }
        dbg_str(DBG_DETAIL, "close shell ui, __open_ui:%p", __open_ui);
    } CATCH (ret) {
    } FINALLY {
        if (linebuf) free(linebuf);
        if (str) object_destroy(str);
    }

    return ret;
}

static class_info_entry_t shell_class_info[] = {
    Init_Obj___Entry(0 , FShell, parent),
    Init_Nfunc_Entry(1 , UnixFShell, construct, __construct),
    Init_Nfunc_Entry(2 , UnixFShell, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , UnixFShell, load, __load),
    Init_Vfunc_Entry(4 , UnixFShell, unload, __unload),
    Init_Vfunc_Entry(5 , UnixFShell, get_func_addr, __get_func_addr),
    Init_Vfunc_Entry(6 , UnixFShell, get_func_name, __get_func_name),
    Init_Vfunc_Entry(7 , UnixFShell, open_ui, __open_ui),
    Init_Vfunc_Entry(8 , UnixFShell, run_func, NULL),
    Init_Vfunc_Entry(9 , UnixFShell, is_statement, NULL),
    Init_End___Entry(10, UnixFShell),
};
REGISTER_CLASS(UnixFShell, shell_class_info);
#endif
