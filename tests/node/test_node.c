#if (!defined(WINDOWS_USER_MODE))
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/net/bus/bus.h>
#include <libobject/core/utils/registry/registry.h>

int test_node_invoke_exit()
{
    allocator_t *allocator = allocator_get_default_instance();
    bus_t *bus;
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
	char out[1024] = {0};
    int out_len = sizeof(out);
    int ret;

    TRY {
        dbg_str(DBG_VIP,"test_bus_client");
        bus = bus_create(allocator, deamon_host, deamon_srv, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);

        bus_invoke_sync(bus, "node", "exit", 0, NULL, out, &out_len);
    } CATCH (ret) {} FINALLY {
        bus_destroy(bus);
    }

    return ret;
}
REGISTER_TEST_CMD(test_node_invoke_exit);

int test_node_invoke_setloglevel()
{
    allocator_t *allocator = allocator_get_default_instance();
    bus_t *bus;
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
	char out[1024];
    char out_len;
    bus_method_args_t args[3] = {
        [0] = {ARG_TYPE_INT32, "bussiness", "0"}, 
        [1] = {ARG_TYPE_INT32, "switch", "1"}, 
        [2] = {ARG_TYPE_INT32, "level", "6"}, 
    };
    int ret;
    
    TRY {
        bus = bus_create(allocator, deamon_host, deamon_srv, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);

        bus_invoke_sync(bus, "node", "set_loglevel", 3, args, out, &out_len);
        dbg_buf(DBG_DETAIL, "return buffer:", (uint8_t *)out, out_len);
    } CATCH (ret) {} FINALLY {
        bus_destroy(bus);
    }
    dbg_str(DBG_DETAIL, "test_bus_client");

    return ret;
}
REGISTER_TEST_CMD(test_node_invoke_setloglevel);
#endif