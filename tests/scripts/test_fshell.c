#if (!defined(WINDOWS_USER_MODE))
#include <dlfcn.h>
#endif
#include <libobject/core/String.h>
#include <libobject/scripts/fshell/FShell.h>
#include <libobject/mockery/mockery.h>

extern int print_outbound(int a, int b, int c, int d, int e, int f, int *g);

static int test_fshell_open()
{
    int ret;
    FShell *shell;
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
#if (!defined(WINDOWS_USER_MODE))
        shell = object_new(allocator,"UnixFShell", NULL);
#else
        shell = object_new(allocator,"WindowsFShell", NULL);
#endif
        shell->init(shell);
        shell->open_ui(shell);
    } CATCH (ret) {
    } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_CMD(test_fshell_open);

static int test_fshell_get_addr()
{
    int ret;
    FShell *shell;
    char *func_name = "object_new";
    void *expect_addr = object_new;
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
#if (!defined(WINDOWS_USER_MODE))
        shell = object_new(allocator,"UnixFShell", NULL);
#else
        shell = object_new(allocator,"WindowsFShell", NULL);
#endif
        EXEC(shell->get_func_addr(shell, NULL, func_name, &addr));
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "add:%p expec addr:%p", addr, expect_addr);
        THROW_IF(addr != expect_addr, -1);
    } CATCH (ret) {
    } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fshell_get_addr);

static int test_fshell_get_func_name()
{
    int ret;
    FShell *shell;
    void *func_addr = print_outbound;
    char *expect_func_name = "print_outbound";
    char name[20];
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
#if (!defined(WINDOWS_USER_MODE))
        shell = object_new(allocator,"UnixFShell", NULL);
#else
        shell = object_new(allocator,"WindowsFShell", NULL);
#endif
        EXEC(shell->get_func_name(shell, NULL, func_addr, name, 20));
        dbg_str(DBG_DETAIL, "name:%s", name);
        ret = strcmp(expect_func_name, name);
        dbg_str(DBG_VIP, "name:%s, expect_func_name:%s", name, expect_func_name);
        THROW_IF(ret != 0, -1);
    } CATCH (ret) {
    } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fshell_get_func_name);

static int test_fshell_load()
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
#if (!defined(WINDOWS_USER_MODE))
        shell = object_new(allocator,"UnixFShell", NULL);
#else
        shell = object_new(allocator,"WindowsFShell", NULL);
#endif
        system("pwd");
#if (!defined(WINDOWS_USER_MODE))
        EXEC(shell->load(shell, lib_name, RTLD_LOCAL | RTLD_LAZY));
#endif
    } CATCH (ret) {
    } FINALLY {
        object_destroy(shell);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_fshell_load);

static int test_fshell_load_and_get_func_addr()
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
#if (!defined(WINDOWS_USER_MODE))
        shell = object_new(allocator,"UnixFShell", NULL);
#else
        shell = object_new(allocator,"WindowsFShell", NULL);
#endif
        system("pwd");
#if (!defined(WINDOWS_USER_MODE))
        EXEC(shell->load(shell, lib_name, RTLD_LOCAL | RTLD_LAZY));
#endif
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
REGISTER_TEST_FUNC(test_fshell_load_and_get_func_addr);

#if (defined(WINDOWS_USER_MODE))
#include <windows.h>
static int test_windows_dl()
{
    int ret;
    FShell *shell;
    HMODULE handle;
    char *func_name = "test_lib_print_outbound";
    char *lib_name = "./sysroot/windows/lib/libobject-testlib.dll";

    char name[20];
    void *addr;
    allocator_t *allocator = allocator_get_default_instance();

    TRY {
        // handle = LoadLibrary("./sysroot/windows/lib/libobject-core.dll");
        handle = GetModuleHandle(NULL);
        // GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, "libobject-core.dll", &handle);
        THROW_IF(handle == NULL, -1);
        addr = GetProcAddress(handle, "allocator_get_default_instance");
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "%p %p",  allocator_get_default_instance, addr);
    } CATCH (ret) {
    } FINALLY {
        FreeLibrary(handle);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_windows_dl);
#endif