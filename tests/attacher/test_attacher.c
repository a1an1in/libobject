// #if 1
#if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) || defined(IOS_USER_MODE))
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
#include <libobject/argument/Application.h>
#include <libobject/scripts/fshell/FShell.h>

extern int attacher_test_without_arg();
extern int attacher_get_debug_info_address();
extern int attacher_test_without_pointer_arg(int a, int b, int c, int d, int e, int f, long g, long h);
extern int attacher_test_with_pointer_arg(int par1, char *par2);
extern int attacher_test2_with_pointer_arg(int par1, char *par2);
extern int testlib_test();

static int test_attacher_get_remote_function_address_case1(Attacher *attacher, pid_t pid)
{
    int ret;
    char *name = "attacher_test_without_arg";
    char *module_name = "libattacher-builtin.so";
    void *addr;

    TRY {
        addr = attacher->get_remote_function_address(attacher, NULL, "attacher_test_without_arg");
        dbg_str(DBG_VIP, "attacher_test_without_arg:%p, addr:%p", attacher_test_without_arg, addr);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {}

    return ret;
}

/* 测试获取进程自带地址 */
static int test_attacher_get_remote_function_address_case2(Attacher *attacher, pid_t pid)
{
    int ret, return_value;
    char *func_name = "test_with_mixed_type_pars";
    void *addr;

    TRY {
        EXEC(attacher->call(attacher, NULL, "attacher_get_func_addr_by_name(\"test_with_mixed_type_pars\", 0)", &addr));
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "%s addr:%p", func_name, addr);
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {}

    return ret;
}

static int test_attacher_get_remote_function_address_case3(Attacher *attacher, pid_t pid)
{
    int ret;
    char *name = "__libc_dlopen_mode";
    void *addr;

    TRY {
        // 如果函数本地有就可以不用指定库名
        addr = attacher->get_remote_function_address(attacher, NULL, name);
        dbg_str(DBG_WIP, "get_remote_function_address %s:%p", name, addr);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {}

    return ret;
}

static int test_attacher_call_address_without_pars(Attacher *attacher, pid_t pid)
{
    int ret;
    void *func_addr;

    TRY {
        func_addr = attacher->get_remote_function_address(attacher, NULL, "attacher_test_without_arg");
        THROW_IF(func_addr == NULL, -1);
        EXEC(ret = attacher->call_address_with_value_pars(attacher, func_addr, 0, 0));
        THROW_IF(ret != 0xadad, -1);
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY { }

    return ret;
}

static int test_attacher_call_address_with_value_pars(Attacher *attacher, pid_t pid)
{
    int ret;
    void *func_addr;
    long long pars[8] = {1, 2, 3, 4, 5, 7, 0xf1f1f1f1f1f1f1f1, 0xf2};

    TRY {
        attacher_test_without_pointer_arg(1, 1, 2, 2, 5, 6, 0xf1, 0xf2);
        func_addr = attacher->get_remote_function_address(attacher, NULL, "attacher_test_without_pointer_arg");
        THROW_IF(func_addr == NULL, -1);
        EXEC(ret = attacher->call_address_with_value_pars(attacher, func_addr, pars, 8));
        THROW_IF(ret != 0xadad, -1);
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY { }

    return ret;
}

static int test_attacher_call_address_with_pointer_pars(Attacher *attacher, pid_t pid)
{
    int ret;
    void *func_addr;
    attacher_paramater_t pars[2] = {{0x1234, 0}, {"test2", 6}};

    TRY {
        func_addr = attacher->get_remote_function_address(attacher, NULL, "attacher_test_with_pointer_arg");
        THROW_IF(func_addr == NULL, -1);
        dbg_str(DBG_VIP, "func_addr:%p", func_addr);
        EXEC(ret = attacher->call_address(attacher, func_addr, pars, 2));
        THROW_IF(ret != 0xadae, -1);
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY { }

    return ret;
}

static int test_attacher_add_and_remove_lib(Attacher *attacher, pid_t pid)
{
    int ret;
    char *name = "./sysroot/linux/x86_64/lib/libtestlib.so";

    TRY {
        EXEC(attacher->add_lib(attacher, name));
        EXEC(attacher->remove_lib(attacher, name));
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {}

    return ret;
}

static int test_attacher_call_from_adding_lib(Attacher *attacher, pid_t pid)
{
    int ret, return_value;
    char *name = "./sysroot/linux/x86_64/lib/libtestlib.so";

    TRY {
        EXEC(attacher->add_lib(attacher, name));
        EXEC(attacher->call(attacher, "libtestlib.so", "testlib_test()", &return_value));
        THROW_IF(return_value != 0xadad, -1);
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {
        attacher->remove_lib(attacher, name);
    }

    return ret;
}

static int test_attacher_call_directly(Attacher *attacher, pid_t pid)
{
    long ret, return_value;

    TRY {
        dbg_str(DBG_DETAIL, "test_attacher_call_directly start"); 
        EXEC(attacher->call(attacher, NULL, "attacher_test_with_pointer_arg(0x1234, \"test2\")", &return_value));
        THROW_IF(return_value != 0xadae, -1);
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {}

    return ret;
}

static int test_attacher_malloc_and_mfree(Attacher *attacher, pid_t pid)
{
    int ret;
    void *func_addr, *addr;
    long long pars[1] = {8};

    TRY {
        addr = attacher->malloc(attacher, 8, NULL);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "malloc addr:%p", addr);

        EXEC(attacher->free(attacher, addr));
        // THROW_IF(ret != 0xadad, -1);
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY { }

    return ret;
}

static int test_attacher_read_and_write_data(Attacher *attacher, pid_t pid)
{
    int ret, return_value;
    void *addr;
    char buffer[1024] = {0};

    TRY {
        EXEC(attacher->call(attacher, NULL, "attacher_get_debug_info_address()", &addr));
        THROW_IF(addr == NULL, -1);
        EXEC(attacher->read(attacher, addr, buffer, 20));
        THROW_IF(strcmp("hello world", buffer) != 0, -1);

        strcpy(buffer, "hello world2");
        EXEC(attacher->write(attacher, addr, buffer, 20));
        memset(buffer, 0, sizeof(buffer));
        EXEC(attacher->read(attacher, addr, buffer, 20));
        THROW_IF(strcmp("hello world2", buffer) != 0, -1);
        
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY { }

    return ret;
}

static int test_attacher_call_stub(Attacher *attacher, pid_t pid)
{
    int ret, return_value;
    stub_t *stub;

    TRY {
        EXEC(attacher->alloc_stub(attacher, &stub));
        EXEC(attacher->add_stub_hooks(attacher, stub, "attacher_test_with_pointer_arg", NULL, 
                                      "attacher_test2_with_pointer_arg", NULL, 2));
        EXEC(attacher->call(attacher, NULL, "attacher_test_with_pointer_arg(0x1234, \"test2\")", &return_value));
        THROW_IF(return_value != 0xadaf, -1);
        dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {
        attacher->remove_stub_hooks(attacher, stub);
        attacher->free_stub(attacher, stub);
    }

    return ret;
}

/* 等待子进程完成 exec + 动态链接: exec 后立刻 attach 可能撞在 ld.so
 * 还没把 libc.so.6/libattacher-builtin.so 映射进子进程的中途,
 * /proc/<pid>/maps 查不到模块, 远端符号解析随机会失败
 * (docker 全量跑时偶发, 单跑难复现).
 * 注意 fork 出的子进程在 execv 之前, maps 是父进程地址空间的写拷贝,
 * libc/libattacher-builtin 子串也在其中, 会造成"已就绪"的假象, 因此
 * 还要确认 exe 本体(test-process)已映射才算就绪. 轮询至多 timeout_ms
 * 毫秒: 就绪返回 0, 超时/子进程消失返回 -1. */
static int __wait_remote_process_started(pid_t pid, int timeout_ms)
{
    char maps[64];
    char line[512];
    int polls;

    snprintf(maps, sizeof(maps), "/proc/%d/maps", pid);

    for (polls = 0; polls < timeout_ms / 10; polls++) {
        FILE *fp;
        int have_libc = 0, have_builtin = 0, have_exe = 0;

        fp = fopen(maps, "r");
        if (fp == NULL) {
            return -1;
        }
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "libc.so.6")) {
                have_libc = 1;
            }
            if (strstr(line, "libattacher-builtin.so")) {
                have_builtin = 1;
            }
            if (strstr(line, "test-process")) {
                have_exe = 1;
            }
        }
        fclose(fp);

        if (have_libc && have_builtin && have_exe) {
            return 0;
        }
        usleep(10000);
    }

    return -1;
}

static int test_attacher(TEST_ENTRY *entry, int argc, void **argv)
{
    Application *app = get_global_application();
    allocator_t *allocator = allocator_get_default_instance();
    FShell *shell;
    pid_t pid = -1;
#   if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE))
    char *path = "./sysroot/linux/x86_64/bin/test-process";
#   elif (defined(WINDOWS_USER_MODE))
    char *path = "./sysroot/windows/x86_64/bin/test-process";
#   elif (defined(MAC_USER_MODE) || defined(IOS_USER_MODE))
    char *path = "./sysroot/mac/bin/test-process";
#   else
    char *path = "./sysroot/linux/x86_64/bin/test-process";
#   endif
    char *arg_vector[2] = {"test-process", NULL};
    Attacher *attacher = NULL;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "test_attacher");
        EXEC((pid = process_execv(path, arg_vector)));
        dbg_str(DBG_VIP, "test_attacher pid:%d", pid);

        /* execv 后先等子进程完成 exec + 动态链接再 attach(见函数注释) */
        EXEC(__wait_remote_process_started(pid, 2000));

        attacher = object_new(allocator, "UnixAttacher", NULL);
        EXEC(attacher->attach(attacher, pid));
        EXEC(attacher->init(attacher));

        EXEC(test_attacher_get_remote_function_address_case1(attacher, pid));
        EXEC(test_attacher_get_remote_function_address_case2(attacher, pid));
        // EXEC(test_attacher_get_remote_function_address_case3(attacher, pid));
        EXEC(test_attacher_call_address_without_pars(attacher, pid));
        EXEC(test_attacher_call_address_with_value_pars(attacher, pid));
        EXEC(test_attacher_call_address_with_pointer_pars(attacher, pid));
        EXEC(test_attacher_add_and_remove_lib(attacher, pid));
        EXEC(test_attacher_call_from_adding_lib(attacher, pid));
        EXEC(test_attacher_call_directly(attacher, pid));
        EXEC(test_attacher_malloc_and_mfree(attacher, pid));
        EXEC(test_attacher_read_and_write_data(attacher, pid));
        EXEC(test_attacher_call_stub(attacher, pid));
    } CATCH (ret) { } FINALLY {
        object_destroy(attacher);
        usleep(1000);
        process_kill(pid);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_attacher);
#endif