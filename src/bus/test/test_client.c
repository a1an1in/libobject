#include <libobject/core/utils/dbg/debug.h>
#include <libobject/bus/bus.h>

#if 1
void test_bus_client()
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
	char out[1024] = {0};
    char out_len;
    /*
     *char *args[2] = {"abc","hello world!"};
     */
    bus_method_args_t args[2] = {
        [0] = {ARG_TYPE_INT32,"id", "123"},
        [1] = {ARG_TYPE_STRING,"content", "hello_world"},
    };
    
    dbg_str(DBG_DETAIL,"test_bus_client");

    bus = bus_create(allocator,
                     deamon_host,
                     deamon_srv,
                     CLIENT_TYPE_INET_TCP);

    bus_invoke_sync(bus,"test", "hello",2, args,out,&out_len);
    dbg_buf(DBG_DETAIL,"return buffer:",(uint8_t *)out, out_len);

    bus_destroy(bus);
	
}
#else
void test_bus_client()
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
    uint8_t out_len;
    /*
     *char *args[2] = {"abc","hello world!"};
     */
    bus_method_args_t args[2] = {
        [0] = {ARG_TYPE_INT32,"id", "123"},
        [1] = {ARG_TYPE_STRING,"content", "hello_world"},
    };
    
    dbg_str(DBG_DETAIL,"test_bus_client");

    bus = bus_create(allocator,
                     deamon_host,
                     deamon_srv,
                     CLIENT_TYPE_INET_TCP);

    bus_lookup_sync(bus, "test");

    bus_destroy(bus);
	
}
#endif
