#if (!defined(WINDOWS_USER_MODE))

#include <libobject/mockery/mockery.h>
#include <libobject/node/Node.h>
#include <libobject/core/io/file_system_api.h>

static int __test_node_bus_call(Node *node)
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

        EXEC(node->list(node, "node", path, list));
        list->for_each(list, fs_file_info_struct_custom_print);
        THROW_IF(list->count(list) != 4, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}

static int __test_node_read(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char from[1024] = "node:./tests/node/res/";
    char to[1024] = "./tests/node/output/read/";
    int ret;
    
    TRY {
        EXEC(fs_rmdir(to));
        EXEC(node->copy(node, from, to));
        strcpy(from, "./tests/node/res/");
        strcat(from, "test_node2.txt");
        strcat(to, "test_node2.txt");
        THROW_IF(assert_file_equal(from, to) != 1, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __test_node_write(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char from[1024] = "./tests/node/res/";
    char to[1024] = "node:./tests/node/output/write/";
    int ret;
    
    TRY {
        EXEC(fs_rmdir("./tests/node/output/write/"));
        EXEC(node->copy(node, from, to));
        strcat(from, "test_node2.txt");
        strcpy(to, "./tests/node/output/write/");
        strcat(to, "test_node2.txt");
        THROW_IF(assert_file_equal(from, to) != 1, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __test_node_fshell(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    int ret;
    
    TRY {
        EXEC(node->call_bus(node, "node@open_fshell()", NULL, 0));
        EXEC(node->execute_fsh(node, "node@fsh_add(1, 2)", NULL, NULL));
        EXEC(node->call_bus(node, "node@close_fshell()", NULL, 0));

        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {}

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
        EXEC(deamon->init(deamon));

        /* 构造测试node */
        node = object_new(allocator, "Node", NULL);
        THROW_IF(node == NULL, -1);
        node->host = deamon_host;
        node->service = deamon_srv;
        node->disable_node_service_flag = 1;
        EXEC(node->init(node));

        // EXEC(__test_node_bus_call(node));
        // EXEC(__test_node_list(node));
        // EXEC(__test_node_read(node));
        // EXEC(__test_node_write(node));
        EXEC(__test_node_fshell(node));
    } CATCH (ret) {} FINALLY {
        object_destroy(node);
        object_destroy(deamon);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_node);

#endif