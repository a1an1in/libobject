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
    char out_len;
    bus_method_args_t args[2] = {
        [0] = {ARG_TYPE_INT32,"id", "123"},
        [1] = {ARG_TYPE_STRING,"content", "hello_world"},
    };
    int ret;

    TRY {
        dbg_str(DBG_VIP,"test_bus_client");
        bus = bus_create(allocator,
                        deamon_host,
                        deamon_srv,
                        CLIENT_TYPE_INET_TCP);

        bus_invoke_sync(bus,"test", "hello",2, args,out,&out_len);
        dbg_buf(DBG_VIP,"return buffer:",(uint8_t *)out, out_len);
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
#if 0
    char *deamon_host = "bus_server_path";
    char *deamon_srv = NULL;
#else
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
#endif
	char out[1024];
    uint8_t out_len;
    /*
     *char *args[2] = {"abc","hello world!"};
     */
    bus_method_args_t args[2] = {
        [0] = {ARG_TYPE_INT32,"id", "123"},
        [1] = {ARG_TYPE_STRING,"content", "hello_world"},
    };
    
    dbg_str(DBG_VIP,"test_bus_client");

    bus = bus_create(allocator,
                     deamon_host,
                     deamon_srv,
                     CLIENT_TYPE_INET_TCP);

    bus_lookup_sync(bus, "test");

    bus_destroy(bus);
	
    return 1;
}
REGISTER_TEST_CMD(test_bus_client_lookup_sync);
#endif