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
#include <libobject/attacher/dynamic_lib.h>

extern int print_outbound(int a, int b, int c, int d, int e, int f, int *g);
extern int test_lib_hello_world();

int test_get_func_addr(TEST_ENTRY *entry)
{
    char *func_name = "print_outbound";
    void *expect_addr = print_outbound;
    void *addr;
    int ret;

    TRY {
        addr = dl_get_func_addr_by_name(func_name);
        THROW_IF(addr == NULL, -1);
        THROW_IF(addr != expect_addr, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_get_func_addr, addr=%p, %s:%p",
                addr, func_name, expect_addr);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_get_func_addr);

int test_dl_get_dynamic_lib_base_address(TEST_ENTRY *entry, int argc, void **argv)
{
    char *func_name = "test_lib_hello_world";
    void *expect_addr = test_lib_hello_world;
    void *addr;
    pid_t pid;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);

        pid = atoi(argv[1]);
        addr = dl_get_dynamic_lib_base_address(-1, "libobject-testlib.so");
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "addr:%p, expect addr:%p", addr, test_lib_hello_world);
        sleep(1000);
        THROW_IF(addr != expect_addr, -1);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_get_func_addr, addr=%p, %s:%p",
                addr, func_name, expect_addr);
    }

    return ret;
}
REGISTER_TEST_CMD(test_dl_get_dynamic_lib_base_address);

int test_dl_get_remote_function_adress(TEST_ENTRY *entry, int argc, void **argv)
{
    char *func_name = "test_lib_hello_world";
    void *expect_addr = test_lib_hello_world;
    void *addr;
    pid_t pid;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "argc:%d", argc);
        for (int i = 0; i < argc; i++) {
            dbg_str(DBG_VIP, "argv[%d]:%s", i, argv[i]);
        }
        THROW_IF(argc != 2, -1);

        pid = atoi(argv[1]);
        addr = dl_get_remote_function_adress(pid, "libobject-testlib.so", test_lib_hello_world);
        THROW_IF(addr == NULL, -1);
        dbg_str(DBG_VIP, "test_dl_get_remote_function_adress local:%p, remote addr:%p", test_lib_hello_world, addr);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_get_func_addr, addr=%p, %s:%p",
                addr, func_name, expect_addr);
    }

    return ret;
}
REGISTER_TEST_CMD(test_dl_get_remote_function_adress);


static int tree_node_free_callback(allocator_t *allocator, void *value)
{
    dbg_str(DBG_VIP, "tree_node_free_callback");
    interval_tree_node_t *t = (interval_tree_node_t *)value;
    allocator_mem_free(allocator, t->value);
    allocator_mem_free(allocator, t);
    return 0;
}

static int test_dl_get_dynamic_name(TEST_ENTRY *entry, int argc, void **argv)
{
    Interval_Tree *tree;
    allocator_t *allocator = allocator_get_default_instance();
    char module_name[64] = {0};
    void *addr;
    pid_t pid;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "argc:%d", argc);
        tree = object_new(allocator, "Interval_Tree", NULL);
        tree->set_trustee(tree, VALUE_TYPE_STRUCT_POINTER, tree_node_free_callback);

        EXEC(dl_parse_dynamic_table(-1, tree));
        EXEC(dl_get_dynamic_lib_name_from_interval_tree(tree, test_lib_hello_world, module_name, 64));

        THROW_IF(strcmp(module_name, "libobject-testlib.so") != 0, -1);
        dbg_str(DBG_VIP, "module_name:%s", module_name);
    } CATCH (ret) { } FINALLY {
        object_destroy(tree);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_dl_get_dynamic_name);

#endif

