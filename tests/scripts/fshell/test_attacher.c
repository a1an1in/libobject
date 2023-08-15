#if (!defined(WINDOWS_USER_MODE))
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>
#include <libobject/scripts/fshell/Attacher.h>

extern int test_lib_hello_world();
extern int test_lib_hello_world_with_value_pars(int a, int b, int c, int d, int e, int f, long g, long h);
extern int test_lib_hello_world_with_pointer_pars(void *par1, void *par2);

/* 测试attacher， 先要运行 test-attach-target进程， 然后获取pid, 执行如下命令进行测试：
 * sudo ./sysroot/linux/bin/xtools mockery --log-level=6 test_attacher_call_without_pars 10334
 */

static int test_attacher_get_function_address(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    char *name = "test_lib_hello_world";
    char *module_name = "libobject-testlib.so";
    Attacher *attacher;
    pid_t pid;

    TRY {
        dbg_str(DBG_SUC, "test_attacher");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);

        pid = atoi(argv[1]);
        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));
        EXEC(attacher->get_function_address(attacher, test_lib_hello_world, module_name));
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_get_function_address);

static int test_attacher_call_without_pars(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *func_addr;
    pid_t pid;

    TRY {
        dbg_str(DBG_SUC, "test_attacher_call_without_pars");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);

        pid = atoi(argv[1]);
        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));
        func_addr = attacher->get_function_address(attacher, test_lib_hello_world, "libobject-testlib.so");
        THROW_IF(func_addr == NULL, -1);
        EXEC(ret = attacher->call(attacher, func_addr, 0, 0));
        THROW_IF(ret != 0xadad, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_call_without_pars);


static int test_attacher_call_with_value_pars(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *func_addr;
    pid_t pid;
    long long pars[8] = {1, 2, 3, 4, 5, 6, 0xf1f1f1f1f1f1f1f1, 0xf2};

    TRY {
        dbg_str(DBG_SUC, "test_attacher_call_with_pars");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);

        test_lib_hello_world_with_value_pars(1, 1, 2, 2, 5, 6, 0xf1, 0xf2);
        pid = atoi(argv[1]);
        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));
        func_addr = attacher->get_function_address(attacher, test_lib_hello_world_with_value_pars, "libobject-testlib.so");
        THROW_IF(func_addr == NULL, -1);
        EXEC(ret = attacher->call(attacher, func_addr, pars, 8));
        THROW_IF(ret != 0xadad, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_call_with_value_pars);

static int test_attacher_call_with_pointer_pars(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *func_addr;
    pid_t pid;
    long long pars[8] = {1, 2, 3, 4, 5, 6, 0xf1f1f1f1f1f1f1f1, 0xf2};

    TRY {
        dbg_str(DBG_SUC, "test_lib_hello_world_with_pointer_pars");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);
        pid = atoi(argv[1]);

        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));
        func_addr = attacher->get_function_address(attacher, test_lib_hello_world_with_pointer_pars, "libobject-testlib.so");
        THROW_IF(func_addr == NULL, -1);
        EXEC(ret = attacher->call(attacher, func_addr, pars, 8));
        THROW_IF(ret != 0xadad, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_call_with_pointer_pars);
#endif

