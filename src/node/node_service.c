#include <unistd.h>
#include <libobject/net/bus/bus.h>
#include <libobject/net/bus/busd.h>
#include <libobject/core/utils/registry/registry.h>
#include "Node.h"

static int node_exit(bus_object_t *obj, int argc,
		             struct blob_attr_s **args,
                     void *out_data, int *out_data_len)
{
    bus_t *bus = obj->bus;
    Node *node = bus->opaque;

    *out_data_len = 0;
    node->node_flag = 1;
    dbg_str(DBG_VIP, "exit node!");
    
	return 1;
}

static const struct blob_policy_s set_loglevel_policy[] = {
	[0] = { .name = "bussiness", .type = BLOB_TYPE_UINT32 }, 
	[1] = { .name = "switch", .type = BLOB_TYPE_UINT32 }, 
	[2] = { .name = "level", .type = BLOB_TYPE_UINT32 }, 
};

static int node_set_loglevel(bus_object_t *obj, int argc, 
		      		         struct blob_attr_s **args, 
                             void *out_data, int *out_data_len)
{
    char buffer[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int db_bussiness, db_switch, db_level;

    db_bussiness = blob_get_uint32(args[0]);
    db_switch    = blob_get_uint32(args[1]);
    db_level     = blob_get_uint32(args[2]);

    debugger_set_business(debugger_gp, db_bussiness, db_switch, db_level);
    dbg_str(DBG_SUC, "set debug, bussiness=%d, switch=%d, level=%d", 
            db_bussiness, db_switch, db_level);

    memcpy(out_data, buffer, sizeof(buffer));
    *out_data_len = sizeof(buffer);

	return 1;
}

static const struct blob_policy_s write_file_policy[] = {
	[0] = { .name = "filename",  .type = BLOB_TYPE_STRING }, 
	[1] = { .name = "buffer",    .type = BLOB_TYPE_BUFFER }, 
    [2] = { .name = "crc32",     .type = BLOB_TYPE_UINT32 }, 
};

static int node_write_file(bus_object_t *obj, int argc, 
		      		       struct blob_attr_s **args, 
                           void *out_data, int *out_data_len)
{
    char *filename;
    uint8_t *buffer;
    uint32_t len, crc32;

    filename = blob_get_string(args[0]);
    len = blob_get_buffer(args[1], &buffer);
    crc32 = blob_get_uint32(args[2]);
    dbg_str(DBG_VIP, "file name:%s, data len:%d, crc32:%d", filename, len, crc32);
    dbg_buf(DBG_VIP, "buffer:", buffer, len);

	return 1;
}

static const struct bus_method node_service_methods[] = {
	BUS_METHOD_WITHOUT_ARG("exit", node_exit, NULL),
    BUS_METHOD("set_loglevel", node_set_loglevel, set_loglevel_policy),
    BUS_METHOD("write_file", node_write_file, write_file_policy),
};

struct bus_object node_object = {
	.id        = (char *)"node",
    .cname     = (char *)"node_service", 
	.methods   = (struct bus_method *)node_service_methods,
	.n_methods = ARRAY_SIZE(node_service_methods),
};
