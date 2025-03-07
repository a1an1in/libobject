#if (!defined(WINDOWS_USER_MODE))
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/net/bus/bus.h>

static const struct blob_policy_s debug_policy[] = {
	[0] = { .name = "bussiness", .type = BLOB_TYPE_UINT32 }, 
	[1] = { .name = "switch", .type = BLOB_TYPE_UINT32 }, 
	[2] = { .name = "level", .type = BLOB_TYPE_UINT32 }, 
};

static int set_debug(bus_object_t *obj, 
                     int argc, 
		      		 struct blob_attr_s **args, 
                     void *out_data, 
                     int *out_data_len)
{
    char buffer[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int db_bussiness, db_switch, db_level;

    db_bussiness = blob_get_uint32(args[0]);
    db_switch    = blob_get_uint32(args[1]);
    db_level     = blob_get_uint32(args[2]);

    dbg_str(DBG_VIP, "set debug, bussiness=%d, switch=%d, level=%d", 
            db_bussiness, db_switch, db_level);

    debugger_set_business(debugger_gp, db_bussiness, db_switch, db_level);

    memcpy(out_data, buffer, sizeof(buffer));
    *out_data_len = sizeof(buffer);

	return 1;
}

static const struct bus_method test_methods[] = {
	BUS_METHOD("set_debug", set_debug, debug_policy), 
};

static bus_object_t debug_object = {
	.methods   = (struct bus_method *)test_methods, 
	.n_methods = ARRAY_SIZE(test_methods), 
};

static void bus_debug_service()
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
    
    bus = bus_create(allocator, 
                     deamon_host, 
                     deamon_srv, 
                     CLIENT_TYPE_INET_TCP);
    memcpy(debug_object.id, "debug", strlen("debug"));
	bus_add_object(bus, &debug_object);

}
#endif