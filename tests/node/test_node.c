#if (!defined(WINDOWS_USER_MODE))
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/net/bus/bus.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/node/Node.h>
#include <libobject/mockery/mockery.h>

static int __test_node_bus_call(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char method[1024] = "node@set_loglevel(1,2,3)";
    int ret;
    
    TRY {
        EXEC((ret = node->call(node, method, NULL, 0)));
        THROW_IF(ret != 1, -1);
    } CATCH (ret) {} FINALLY { }

    return ret;
}

static int __test_node_list(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char path[1024] = "./tests/node/";

    Vector *list;
    int ret;
    
    TRY {
        list = object_new(allocator, "Vector", NULL);

        EXEC(node->list(node, "node", path, list));
        list->for_each(list, fs_file_info_struct_custom_print);
        THROW_IF(list->count(list) != 4, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(list);
    }

    return ret;
}

static int __test_node_read(Node *node)
{
    allocator_t *allocator = allocator_get_default_instance();
    char from[1024] = "node:./tests/node/";
    int ret;
    
    TRY {
        EXEC(node->copy(node, from, "./tests/node/output/"));
    } CATCH (ret) {} FINALLY {
    }

    return ret;
}

static int test_node(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
	Node *node;
    int ret;
    
    TRY {
        node = object_new(allocator, "Node", NULL);
        THROW_IF(node == NULL, -1);
        node->host = deamon_host;
        node->service = deamon_srv;
        node->disable_node_service_flag = 1;
        EXEC(node->init(node));

        // EXEC(__test_node_bus_call(node));
        // EXEC(__test_node_list(node));
        EXEC(__test_node_read(node));
    } CATCH (ret) {} FINALLY {
        object_destroy(node);
    }

    return ret;
}
REGISTER_TEST_CMD(test_node);
#endif