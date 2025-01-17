#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <libobject/core/utils/process.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/core/try.h>
#include <libobject/attacher/Attacher.h>

extern int test_lib_hello_world();
extern int test_lib_get_debug_info_address();
extern int test_lib_hello_world_without_pointer_pars(int a, int b, int c, int d, int e, int f, long g, long h);
extern int test_lib_hello_world_with_pointer_pars(char *par1, char *par2);
extern int test_lib2_hello_world();
extern int test_lib_hello_world_with_pointer_pars2(int par1, char *par2);
extern int test_lib_hello_world_with_pointer_pars3(int par1, char *par2);
extern int stub_hello_world();
extern void *my_malloc(int size);

// #if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) || defined(IOS_USER_MODE))
#if 1

/* 测试attacher， 先要运行 test-attach-target进程， 然后获取pid, 执行如下命令进行测试：
 * sudo ./sysroot/linux/bin/xtools mockery --log-level=6 test_attacher_call_address_without_pars 10334
 */

static int test_attacher_get_function_address(Attacher *attacher, pid_t pid)
{
    int ret;
    char *name = "test_lib_hello_world";
    char *module_name = "libobject-testlib.so";
    void *addr;

    TRY {
        addr = attacher->get_function_address(attacher, test_lib_hello_world, module_name);
        dbg_str(DBG_VIP, "test_lib_hello_world:%p, addr:%p", test_lib_hello_world, addr);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {}

    return ret;
}

static int test_attacher_call_address_without_pars(Attacher *attacher, pid_t pid)
{
    int ret;
    void *func_addr;

    TRY {
        func_addr = attacher->get_function_address(attacher, test_lib_hello_world, "libobject-testlib.so");
        THROW_IF(func_addr == NULL, -1);
        EXEC(ret = attacher->call_address_with_value_pars(attacher, func_addr, 0, 0));
        THROW_IF(ret != 0xadad, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY { }

    return ret;
}

static int test_attacher_call_address_with_value_pars(Attacher *attacher, pid_t pid)
{
    int ret;
    void *func_addr;
    long long pars[8] = {1, 2, 3, 4, 5, 6, 0xf1f1f1f1f1f1f1f1, 0xf2};

    TRY {
        test_lib_hello_world_without_pointer_pars(1, 1, 2, 2, 5, 6, 0xf1, 0xf2);
        func_addr = attacher->get_function_address(attacher, test_lib_hello_world_without_pointer_pars, "libobject-testlib.so");
        THROW_IF(func_addr == NULL, -1);
        EXEC(ret = attacher->call_address_with_value_pars(attacher, func_addr, pars, 8));
        THROW_IF(ret != 0xadad, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY { }

    return ret;
}

static int test_attacher_call_address_malloc(Attacher *attacher, pid_t pid)
{
    int ret;
    void *func_addr, *addr;
    long long pars[1] = {8};

    TRY {
        addr = attacher->malloc(attacher, 8, NULL);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "malloc addr:%p", addr);

        attacher->free(attacher, addr);
        dbg_str(DBG_VIP, "free addr:%p", addr);
        
        // THROW_IF(ret != 0xadad, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY { }

    return ret;
}

static int test_attacher_call_address_with_pointer_pars(Attacher *attacher, pid_t pid)
{
    int ret;
    void *func_addr;
    attacher_paramater_t pars[2] = {{"test1", 8}, {"test2", 8}};

    TRY {
        func_addr = attacher->get_function_address(attacher, test_lib_hello_world_with_pointer_pars, "libobject-testlib.so");
        THROW_IF(func_addr == NULL, -1);
        dbg_str(DBG_VIP, "func_addr:%p", func_addr);
        EXEC(ret = attacher->call_address(attacher, func_addr, pars, 2));
        // THROW_IF(ret != 0xadad, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY { }

    return ret;
}

static int test_attacher_call_from_lib(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *func_addr;
    pid_t pid;
    attacher_paramater_t pars[2] = {{"test1", 6}, {"test2", 6}};

    TRY {
        dbg_str(DBG_VIP, "test_attacher_call_from_lib");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);
        pid = atoi(argv[1]);

        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));

        EXEC(ret = attacher->call_from_lib(attacher, "test_lib_hello_world_with_pointer_pars", pars, 2, "libobject-testlib.so"));
        // THROW_IF(ret != 0xadad, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_call_from_lib);

static int test_attacher_call_from_lib2(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *func_addr;
    pid_t pid;
    attacher_paramater_t pars[2] = {{0x1234, 0}, {"test2", 6}};

    TRY {
        dbg_str(DBG_VIP, "test_attacher_call_from_lib");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);
        pid = atoi(argv[1]);

        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));

        EXEC(ret = attacher->call_from_lib(attacher, "test_lib_hello_world_with_pointer_pars2", pars, 2, "libobject-testlib.so"));
        // THROW_IF(ret != 0xadad, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_call_from_lib2);

static int test_attacher_add_lib(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *func_addr;
    char *name = "/home/alan/workspace/libobject/sysroot/linux/lib/libobject-testlib2.so";
    pid_t pid;

    TRY {
        dbg_str(DBG_VIP, "test_attacher_call_from_lib");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);
        pid = atoi(argv[1]);

        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));

        dbg_str(DBG_VIP, "name len:%d", strlen(name));
        EXEC(ret = attacher->add_lib(attacher, name));
        sleep(1000);
        // THROW_IF(ret != 0xadad, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_add_lib);


static int test_attacher_remove_lib(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *func_addr;
    char *name = "/home/alan/workspace/libobject/sysroot/linux/lib/libobject-testlib2.so";
    pid_t pid;

    TRY {
        dbg_str(DBG_VIP, "test_attacher_call_from_lib");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);
        pid = atoi(argv[1]);

        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));

        dbg_str(DBG_VIP, "name len:%d", strlen(name));
        EXEC(attacher->add_lib(attacher, name));
        sleep(5);
        EXEC(attacher->remove_lib(attacher, name));
        // THROW_IF(ret != 0xadad, -1);
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_remove_lib);

static int test_attacher_call_from_adding_lib(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *func_addr;
    char *name = "/home/alan/workspace/libobject/sysroot/linux/lib/libobject-testlib2.so";
    pid_t pid;

    TRY {
        dbg_str(DBG_VIP, "test_attacher_call_from_lib");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);
        pid = atoi(argv[1]);

        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));
        EXEC(attacher->add_lib(attacher, name));
        EXEC(attacher->call_from_lib(attacher, "test_lib2_hello_world", NULL, 0, "libobject-testlib2.so"));
    } CATCH (ret) { } FINALLY {
        attacher->remove_lib(attacher, name);
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_call_from_adding_lib);

static int test_attacher_read_data(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *addr;
    pid_t pid;
    char buffer[1024] = {0};

    TRY {
        dbg_str(DBG_VIP, "test_attacher_call_from_lib");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);
        pid = atoi(argv[1]);

        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));
        addr = attacher->call_from_lib(attacher, "test_lib_get_debug_info_address", NULL, 0, "libobject-testlib.so");
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "addr:%p", addr);
        addr = attacher->read(attacher, addr, buffer, 20);
        dbg_str(DBG_VIP, "read back data:%s", buffer);
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_read_data);

static int test_attacher_call_stub(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *func_addr;
    stub_t *stub;
    pid_t pid;
    attacher_paramater_t pars[2] = {{0x1234, 0}, {"test2", 6}};

    TRY {
        dbg_str(DBG_VIP, "test_attacher_call_from_lib");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);
        pid = atoi(argv[1]);

        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));
        EXEC(attacher->init(attacher));

        THROW_IF(((stub = attacher->alloc_stub(attacher)) == NULL), -1);
        EXEC(attacher->add_stub_hooks(attacher, stub, test_lib_hello_world_with_pointer_pars2, NULL, 
                                      test_lib_hello_world_with_pointer_pars3, NULL, 2));
        EXEC(attacher->run(attacher));
        dbg_str(DBG_VIP, "sleep ...");
        sleep(10);
    } CATCH (ret) { } FINALLY {
        attacher->stop(attacher);
        attacher->remove_stub_hooks(attacher, stub);
        attacher->free_stub(attacher, stub);
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_call_stub);


static int test_attacher_call_directly(TEST_ENTRY *entry, int argc, void **argv)
{
    int ret;
    allocator_t *allocator = allocator_get_default_instance();
    Attacher *attacher;
    void *func_addr;
    stub_t *stub;
    pid_t pid;
    attacher_paramater_t pars[2] = {{0x1234, 0}, {"test2", 6}};

    TRY {
        dbg_str(DBG_VIP, "test_attacher_call_from_lib");
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);
        pid = atoi(argv[1]);

        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));
        EXEC(attacher->init(attacher));

        EXEC(ret = attacher->call(attacher, test_lib_hello_world_with_pointer_pars3, pars, 2));

    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
    }

    return ret;
}
REGISTER_TEST_CMD(test_attacher_call_directly);

static int test_attacher(TEST_ENTRY *entry, int argc, void **argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    pid_t pid = -1;
#   if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE))
    char *path = "./sysroot/linux/bin/test-process";
#   elif (defined(WINDOWS_USER_MODE))
    char *path = "./sysroot/windows/bin/test-process";
#   elif (defined(MAC_USER_MODE) || defined(IOS_USER_MODE))
    char *path = "./sysroot/mac/bin/test-process";
#   else
    char *path = "./sysroot/linux/bin/test-process";
#   endif
    char *arg_vector[2] = {"test-process", NULL};
    Attacher *attacher;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "test_attacher");
        EXEC((pid = process_execv(path, arg_vector)));
        dbg_str(DBG_VIP, "test_attacher pid:%d", pid);

        sleep(1);
        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));

        EXEC(test_attacher_get_function_address(attacher, pid));
        EXEC(test_attacher_call_address_without_pars(attacher, pid));
        EXEC(test_attacher_call_address_with_value_pars(attacher, pid));
        EXEC(test_attacher_call_address_malloc(attacher, pid));
        // EXEC(test_attacher_call_address_with_pointer_pars(attacher, pid));
        
        
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
        process_kill(pid);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_attacher);
#endif