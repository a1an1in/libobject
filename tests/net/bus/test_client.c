#if (!defined(WINDOWS_USER_MODE))
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/net/bus/bus.h>
#include <libobject/core/utils/registry/registry.h>

int test_bus_client_invoke_sync()
{
    allocator_t *allocator = allocator_get_default_instance();
    bus_t *bus;
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
	char out[1024] = {0};
    int out_len = sizeof(out);
    bus_method_args_t args[2] = {
        [0] = {ARG_TYPE_UINT32, "id", 123},
        [1] = {ARG_TYPE_STRING, "content", "hello_world"},
    };
    int ret;

    TRY {
        dbg_str(DBG_VIP,"test_bus_client");
        bus = bus_create(allocator, deamon_host, deamon_srv, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);

        bus_invoke_sync(bus, "test", "hello", 2, args, out, &out_len);
        dbg_buf(DBG_VIP, "return buffer:", (uint8_t *)out, out_len);
    } CATCH (ret) {} FINALLY {
        bus_destroy(bus);
    }

    return ret;
}
REGISTER_TEST_CMD(test_bus_client_invoke_sync);

int test_bus_client_lookup_sync()
{
    allocator_t *allocator = allocator_get_default_instance();
    bus_t *bus;
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
	char out[1024];
    int out_len = 1024;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "test_bus_client");

        bus = bus_create(allocator, deamon_host, deamon_srv, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);

        bus_lookup_sync(bus, "test", out, &out_len);
    } CATCH (ret) {} FINALLY {
        bus_destroy(bus);
    }
	
    return ret;
}
REGISTER_TEST_CMD(test_bus_client_lookup_sync);

#endif