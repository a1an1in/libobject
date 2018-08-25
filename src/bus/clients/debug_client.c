#include <libobject/core/utils/dbg/debug.h>
#include <libobject/bus/bus.h>

void bus_debug_client(char *bussiness, char *sw, char *level)
{
    allocator_t *allocator = allocator_get_default_alloc();
    bus_t *bus;
#if 0
    char *deamon_host = "bus_server_path";
    char *deamon_srv = NULL;
#else
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
#endif
	char out[1024];
    char out_len;

    bus_method_args_t args[3] = {
        [0] = {ARG_TYPE_INT32, "bussiness", bussiness}, 
        [1] = {ARG_TYPE_INT32, "switch", sw}, 
        [2] = {ARG_TYPE_INT32, "level", level}, 
    };
    
    dbg_str(DBG_DETAIL, "test_bus_client");

    bus = bus_create(allocator, 
                     deamon_host, 
                     deamon_srv, 
                     CLIENT_TYPE_INET_TCP);
    /*
     *bus_lookup(bus, "test");
     */

    /*
     *bus_invoke(bus, "test", "hello", 2, args);
     */

    bus_invoke_sync(bus, "debug", "set_debug", 3, args, out, &out_len);
    dbg_buf(DBG_DETAIL, "return buffer:", (uint8_t *)out, out_len);
}
