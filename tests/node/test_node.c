#include <libobject/mockery/mockery.h>
#include <libobject/node/Node.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/core/utils/byteorder.h>

extern allocator_t *global_allocator_default;
extern int test_print_inbound(int a, int b, int c, int d, int e, int f, int *g);
extern int test_func(int a, int b, int c, int d, int e, int f, int *g);
extern int test_target_func(int a, int b, int c, int d, int e, int f, int *g);
extern int test_print_outbound(int a, int b, int c, int d, int e, int f, int *g);

static int __test_node_call_bus(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char method[1024] = "node@set_loglevel(1,2,3)";
    int ret;
    
    TRY {
        EXEC((ret = node->call_bus(node, method, NULL, 0)));
        THROW_IF(ret != 1, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY { }

    return ret;
}

static int __test_node_list(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char path[1024] = "./tests/node/res/";
    Vector *list;
    int ret;
    
    TRY {
        list = object_new(allocator, "Vector", NULL);

        EXEC(node->flist(node, "node", path, list));
        list->for_each(list, fs_file_info_struct_custom_print);
        THROW_IF(list->count(list) != 4, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}

static int __test_node_read_file(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char from[1024] = "node@./tests/node/res/test_node.txt";
    char to[1024] = "./tests/node/output/read/test_node.txt";
    int ret;
    
    TRY {
        EXEC(fs_rmdir("./tests/node/output/read/"));
        EXEC(node->fcopy(node, from, to));
        THROW_IF(assert_file_equal("./tests/node/res/test_node.txt", to) != 1, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __test_node_read_files(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char from[1024] = "node@./tests/node/res/";
    char to[1024] = "./tests/node/output/read/";
    int ret;
    
    TRY {
        EXEC(fs_rmdir(to));
        EXEC(node->fcopy(node, from, to));
        strcpy(from, "./tests/node/res/");
        strcat(from, "test_node2.txt");
        strcat(to, "test_node2.txt");
        THROW_IF(assert_file_equal(from, to) != 1, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __test_node_write_file(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char from[1024] = "./tests/node/res/";
    char to[1024] = "node@./tests/node/output/write/";
    int ret;
    
    TRY {
        EXEC(fs_rmdir("./tests/node/output/write/"));
        EXEC(node->fcopy(node, from, to));
        THROW_IF(assert_file_equal("./tests/node/res/test_node.txt", "./tests/node/output/write/test_node.txt") != 1, -1);
        THROW_IF(assert_file_equal("./tests/node/res/test_node2.txt", "./tests/node/output/write/test_node2.txt") != 1, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __test_node_write_files(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char from[1024] = "./tests/node/res/";
    char to[1024] = "node@./tests/node/output/write/";
    int ret;
    
    TRY {
        EXEC(fs_rmdir("./tests/node/output/write/"));
        EXEC(node->fcopy(node, from, to));
        strcat(from, "test_node2.txt");
        strcpy(to, "./tests/node/output/write/");
        strcat(to, "test_node2.txt");
        THROW_IF(assert_file_equal(from, to) != 1, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __test_node_malloc_and_mfree(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    void *addr = NULL;
    int ret;
    
    TRY {
        EXEC(node->malloc(node, "node", TARGET_TYPE_NODE, VALUE_TYPE_ALLOC_POINTER, NULL, "#abc", 8, &addr));
        dbg_str(DBG_SUC, "node alloc addr:%p", addr);
        EXEC(node->mfree(node, "node", TARGET_TYPE_NODE, "#abc"));

        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {}

    return ret;
}

static int __test_node_mset_and_mget(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    void *addr = NULL;
    char *test_value = "hello world";
    char *variable_name = "#test_abc";
    char buffer[1024] = {0};
    int ret, len = strlen(test_value);
    
    TRY {
        strcpy(buffer, "hello world");
        EXEC(node->malloc(node, "node", TARGET_TYPE_NODE, VALUE_TYPE_ALLOC_POINTER, NULL, variable_name, 1024, &addr));
        EXEC(node->mset(node, "node", TARGET_TYPE_NODE, addr, 0, 1024, test_value, strlen(test_value)));
        memset(buffer, 0, sizeof(buffer));
        EXEC(node->mget(node, "node", TARGET_TYPE_NODE, addr, 0, 1024, buffer, &len));
        THROW_IF(len != strlen(test_value), -1);
        THROW_IF(strcmp(test_value, buffer) != 0, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {
        node->mfree(node, "node", TARGET_TYPE_NODE, variable_name);
    }

    return ret;
}

static int __test_node_call_fsh(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    void *addr = NULL;
    char *variable_name1 = "#test_stub_v1";
    char buffer[1024];
    int ret, len = sizeof(int);
    
    TRY {
        EXEC(node->malloc(node, "node", TARGET_TYPE_NODE, VALUE_TYPE_ALLOC_POINTER, NULL, variable_name1, sizeof(int), &addr));
        EXEC(node->call_fsh(node, "node@fsh_test_add_v2(%d, %d, 0x%p)", 1, 2, addr));
        EXEC(node->mget(node, "node", TARGET_TYPE_NODE, addr, 0, sizeof(int), buffer, &len));
        THROW_IF(*(uint32_t *)buffer != 3, -1);

        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {
        node->mfree(node, "node", TARGET_TYPE_NODE, variable_name1);
    }

    return ret;
}

static int __test_node_call_fsh_object_method(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char *object_name = "#test_string";
    char *expect = "abc";
    String *str = NULL;
    int ret; 
    
    TRY {
        EXEC(node->malloc(node, "node", TARGET_TYPE_NODE, VALUE_TYPE_OBJ_POINTER, "String", object_name, 8, &str));
        dbg_str(DBG_SUC, "node alloc object addr:%p", str);
        EXEC(node->call_fsh_object_method(node, "%s@assign(%s, \"%s\")", "node", object_name, expect));
        THROW_IF(strncmp(expect, STR2A(str), strlen(expect)) != 0, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {
        node->mfree(node, "node", TARGET_TYPE_NODE, object_name);
    }

    return ret;
}

static int __test_node_mget_pointer(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    void *addr = NULL;
    int ret;
    
    TRY {
        EXEC(node->mget_pointer(node, "node", TARGET_TYPE_NODE, &global_allocator_default, &addr));
        THROW_IF(addr != global_allocator_default, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY { }

    return ret;
}

static int __test_node_stub(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char *stub_name = "#test_stub_v1";
    int g = 7, len = sizeof(void *);
    int ret;
    
    TRY {
        EXEC(node->malloc(node, "node", TARGET_TYPE_NODE, VALUE_TYPE_STUB_POINTER, NULL, stub_name, 0, NULL));

        /* 因为node 和 node cli 是在同一个进程中， 所有可以直接使用test_func测试，不需要调用node执行test_func。 */
        test_func(1, 2, 3, 4, 5, 6, &g);
        THROW_IF(g != 7, -1);

        EXEC(node->call_fsh(node, "%s@fsh_add_stub_hooks(%s, \"%s\", \"%s\", \"%s\", \"%s\", %d)", 
                            "node", stub_name, "test_func", "test_print_inbound", 
                            "test_target_func", "test_print_outbound", 7));
        test_func(1, 2, 3, 4, 5, 6, &g);
        THROW_IF(g != 8, -1);
        EXEC(node->call_fsh(node, "%s@fsh_remove_stub_hooks(%s)", "node", stub_name));
        g = 7;
        test_func(1, 2, 3, 4, 5, 6, &g);
        THROW_IF(g != 7, -1);

        dbg_str(DBG_SUC, "command suc, func_name = %s, file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) { } FINALLY {
        node->mfree(node, "node", TARGET_TYPE_NODE, stub_name);
    }

    return ret;
}

static int __test_node_lookup(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    Vector *list;
    int ret;
    
    TRY {
        list = object_new(allocator, "Vector", NULL);

        EXEC(node->lookup(node, "node", list));
        THROW_IF(list->count(list) != 1, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}

static int test_node(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
	Node *node, *deamon;
    int ret;
    
    TRY {
        /* 构造node deamon, 这样就可以不依赖其它外部进程 */
        deamon = object_new(allocator, "Node", NULL);
        THROW_IF(deamon == NULL, -1);
        deamon->host = deamon_host;
        deamon->service = deamon_srv;
        deamon->run_bus_deamon_flag = 1;
        /* 为了测试方便需要固定node id */
        deamon->node_id = "node";
        EXEC(deamon->init(deamon));

        /* 构造node cli, 不需要node service */
        node = object_new(allocator, "Node", NULL);
        THROW_IF(node == NULL, -1);
        node->host = deamon_host;
        node->service = deamon_srv;
        node->disable_node_service_flag = 1;
        EXEC(node->init(node));

        EXEC(__test_node_call_bus(node));
        EXEC(__test_node_list(node));
        EXEC(__test_node_read_file(node));
        EXEC(__test_node_read_files(node));
        EXEC(__test_node_write_file(node));
        EXEC(__test_node_write_files(node));
        EXEC(__test_node_malloc_and_mfree(node));
        EXEC(__test_node_mset_and_mget(node));
        EXEC(__test_node_call_fsh(node));
        EXEC(__test_node_call_fsh_object_method(node));
        EXEC(__test_node_mget_pointer(node));
        EXEC(__test_node_stub(node));
        EXEC(__test_node_lookup(node));
    } CATCH (ret) {} FINALLY {
        object_destroy(node);
        usleep(1000);
        object_destroy(deamon);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_node);