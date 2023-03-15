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

static int __construct(UnixFShell *shell, char *init_str)
{
    return 0;
}

static int __deconstruct(UnixFShell *shell)
{
    Map *map = shell->parent.map;

    Iterator *cur, *end;
    void *key, *value;

    cur = map->begin(map);
    end = map->end(map);

    for (; !end->equal(end, cur); cur->next(cur)) {
        key = cur->get_kpointer(cur);
        value = cur->get_vpointer(cur);
        dbg_str(DBG_DETAIL, "unix fshell close lib:%s, handle:%p", key, value);
        dlclose(value);
    }

    return 0;
}

static int __load(UnixFShell *shell, char *lib_name, int flag)
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

static int __unload(UnixFShell *shell, char *lib_name, int flag)
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
                dbg_buf(DBG_DETAIL, "linebuf:", linebuf, strlen(linebuf));
                THROW(-1);
            }
        }
        dbg_str(DBG_DETAIL, "close shell ui");
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
REGISTER_CLASS("UnixFShell", shell_class_info);

extern int print_outbound(int a, int b, int c, int d, int e, int f, int *g);

static int test_unixfshell_open()
{
    int ret;
    FShell *shell;
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
        shell = OBJECT_NEW(allocator, UnixFShell, NULL);
        shell->open_ui(shell);
    } CATCH (ret) {
    } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_unixfshell_open);

static int test_unixfshell_get_addr()
{
    int ret;
    FShell *shell;
    char *func_name = "print_outbound";
    void *expect_addr = print_outbound;
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
        shell = OBJECT_NEW(allocator, UnixFShell, NULL);
        EXEC(shell->get_func_addr(shell, NULL, func_name, &addr));
        THROW_IF(addr == NULL, -1);
        THROW_IF(addr != expect_addr, -1);
    } CATCH (ret) {
    } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_unixfshell_get_addr);

static int test_unixfshell_get_func_name()
{
    int ret;
    FShell *shell;
    void *func_addr = print_outbound;
    char *expect_func_name = "print_outbound";
    char name[20];
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
        shell = OBJECT_NEW(allocator, UnixFShell, NULL);
        EXEC(shell->get_func_name(shell, NULL, func_addr, name, 20));
        dbg_str(DBG_DETAIL, "name:%s", name);
        ret = strcmp(expect_func_name, name);
        THROW_IF(ret != 0, -1);
    } CATCH (ret) {
    } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_unixfshell_get_func_name);

static int test_unixfshell_load()
{
    int ret;
    FShell *shell;
    char *expect_func_name = "test_lib_print_outbound";
    #if (defined(MAC_USER_MODE))
    char *lib_name = "./sysroot/mac/lib/libobject-testlib.dylib";
    #else
    char *lib_name = "./sysroot/linux/lib/libobject-testlib.so";
    #endif
    char name[20];
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
        shell = OBJECT_NEW(allocator, UnixFShell, NULL);
        system("pwd");
        EXEC(shell->load(shell, lib_name, RTLD_LOCAL | RTLD_LAZY));
    } CATCH (ret) {
    } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_unixfshell_load);

static int test_unixfshell_load_and_get_func_addr()
{
    int ret = 0;
    FShell *shell;
    char *func_name = "test_lib_print_outbound";
    #if (defined(MAC_USER_MODE))
    char *lib_name = "./sysroot/mac/lib/libobject-testlib.dylib";
    #else
    char *lib_name = "./sysroot/linux/lib/libobject-testlib.so";
    #endif
    char name[20];
    allocator_t *allocator = allocator_get_default_instance();
    int (*func)(int a, int b, int c, int d, int e, int f, int *g);

    TRY {
        shell = OBJECT_NEW(allocator, UnixFShell, NULL);
        system("pwd");
        EXEC(shell->load(shell, lib_name, RTLD_LOCAL | RTLD_LAZY));
        EXEC(shell->get_func_addr(shell, lib_name, func_name, (void **)&func));
        dbg_str(DBG_DETAIL, "xx  func addr:%p", func);
        func(1, 2, 3, 4, 5, 6, &ret);
        THROW_IF(ret != 123, -1);
    } CATCH (ret) {
    } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_unixfshell_load_and_get_func_addr);

#endif
