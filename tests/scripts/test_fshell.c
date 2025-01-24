#include <dlfcn.h>
#include <libobject/core/String.h>
#include <libobject/scripts/fshell/FShell.h>
#include <libobject/mockery/mockery.h>

extern int test_hello_world();

static int test_fshell_open()
{
    int ret;
    FShell *shell;
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
#if (defined(WINDOWS_USER_MODE))
        shell = object_new(allocator, "WindowsFShell", NULL);
#else
        shell = object_new(allocator, "UnixFShell", NULL);
#endif
        shell->init(shell);
        shell->open_ui(shell);
    } CATCH (ret) { } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_CMD(test_fshell_open);

static int test_fshell_get_addr()
{
    int ret;
    FShell *shell;
    char *func_name = "test_hello_world";
    void *expect_addr = test_hello_world;
    int (*addr)();
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
#if (defined(WINDOWS_USER_MODE))
        shell = object_new(allocator, "WindowsFShell", NULL);
#else
        shell = object_new(allocator, "UnixFShell", NULL); 
#endif
        EXEC(shell->get_func_addr(shell, NULL, func_name, &addr));
        THROW_IF(addr == NULL, -1);

        dbg_str(DBG_VIP, "addr:%p expec addr:%p", addr, expect_addr);
        addr();
        ret = test_hello_world();

#if (defined(WINDOWS_USER_MODE))
        THROW_IF(ret != 2, -1);
#else    
        THROW_IF(addr != expect_addr, -1);
#endif
    } CATCH (ret) { } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fshell_get_addr);

static int test_fshell_get_func_name()
{
    int ret;
    FShell *shell;
    void *func_addr = test_hello_world;
    char *expect_func_name = "test_hello_world";
    char name[20];
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
#if (defined(WINDOWS_USER_MODE))
        shell = object_new(allocator, "WindowsFShell", NULL);
#else
        shell = object_new(allocator, "UnixFShell", NULL);
#endif
        EXEC(shell->get_func_name(shell, NULL, func_addr, name, 20));
        dbg_str(DBG_DETAIL, "name:%s", name);
        ret = strcmp(expect_func_name, name);
        dbg_str(DBG_VIP, "name:%s, expect_func_name:%s", name, expect_func_name);
        THROW_IF(ret != 0, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fshell_get_func_name);

static int test_fshell_load()
{
    int ret;
    FShell *shell;
    char *expect_func_name = "attacher_print_outbound";
#if (defined(MAC_USER_MODE))
    char *lib_name = "./sysroot/mac/lib/attacher-builtin.dylib";
#elif (defined(WINDOWS_USER_MODE))
    char *lib_name = "./sysroot/windows/lib/attacher-builtin.dll";
#else
    char *lib_name = "./sysroot/linux/lib/libattacher-builtin.so";
#endif
    char name[20];
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
#if (defined(WINDOWS_USER_MODE))
        shell = object_new(allocator,"WindowsFShell", NULL);
#else
        shell = object_new(allocator,"UnixFShell", NULL);
#endif
        EXEC(shell->load(shell, lib_name, RTLD_LOCAL | RTLD_LAZY));
    } CATCH (ret) { } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fshell_load);

static int test_fshell_load_and_get_func_addr()
{
    int ret = 0;
    FShell *shell;
    char *func_name = "attacher_print_outbound";
#if (defined(MAC_USER_MODE))
    char *lib_name = "./sysroot/mac/lib/attacher-builtin.dylib";
#elif (defined(WINDOWS_USER_MODE))
    char *lib_name = "./sysroot/windows/lib/attacher-builtin.dll";
#else
    char *lib_name = "./sysroot/linux/lib/libattacher-builtin.so";
#endif
    char name[20];
    allocator_t *allocator = allocator_get_default_instance();
    int (*func)(int a, int b, int c, int d, int e, int f, int *g);

    TRY {
#if (defined(WINDOWS_USER_MODE))
        shell = object_new(allocator, "WindowsFShell", NULL);
#else
        shell = object_new(allocator, "UnixFShell", NULL);
#endif
        EXEC(shell->load(shell, lib_name, RTLD_LOCAL | RTLD_LAZY));
        EXEC(shell->get_func_addr(shell, lib_name, func_name, (void **)&func));
        dbg_str(DBG_DETAIL, "xx  func addr:%p", func);
        func(1, 2, 3, 4, 5, 6, &ret);
        THROW_IF(ret != 123, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fshell_load_and_get_func_addr);