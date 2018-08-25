#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/bus/bus.h>

static const struct blob_policy_s debug_policy[] = {
	[0] = { .name = "bussiness", .type = BLOB_TYPE_INT32 }, 
	[1] = { .name = "switch", .type = BLOB_TYPE_INT32 }, 
	[2] = { .name = "level", .type = BLOB_TYPE_INT32 }, 
};

static int set_debug(struct bus_s *bus, 
                      int argc, 
		      		  struct blob_attr_s **args, 
                      void *out_data, 
                      int *out_data_len)
{
    char buffer[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int db_bussiness, db_switch, db_level;

    db_bussiness = blob_get_u32(args[0]);
    db_switch    = blob_get_u32(args[1]);
    db_level     = blob_get_u32(args[2]);

    dbg_str(DBG_SUC, "set debug, bussiness=%d, switch=%d, level=%d", 
            db_bussiness, db_switch, db_level);

    debugger_set_business(debugger_gp, db_bussiness, db_switch, db_level);

    memcpy(out_data, buffer, sizeof(buffer));
    *out_data_len = sizeof(buffer);

	return 1;
}

static const struct bus_method test_methods[] = {
	BUS_METHOD("set_debug", set_debug, debug_policy), 
};

static struct bus_object debug_object = {
	.name      = (char *)"debug", 
	.methods   = (struct bus_method *)test_methods, 
	.n_methods = ARRAY_SIZE(test_methods), 
};

void bus_debug_service()
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
    
    bus = bus_create(allocator, 
                     deamon_host, 
                     deamon_srv, 
                     CLIENT_TYPE_INET_TCP);

	bus_add_object(bus, &debug_object);

}
