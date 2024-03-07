#include <unistd.h>
#include <libobject/net/bus/bus.h>
#include <libobject/net/bus/busd.h>
#include <libobject/core/io/File.h>
#include <libobject/core/utils/registry/registry.h>
#include "Node.h"

static const struct blob_policy_s test_policy[] = { 
    [0] = { .name = "par1",  .type = BLOB_TYPE_INT32 }, 
};

static int node_test(bus_object_t *obj, int argc, 
		      	     struct blob_attr_s **args, 
                     void *out, int *out_len)
{
    int32_t value;

    value = blob_get_int32(args[0]);
    dbg_str(DBG_VIP, "node_test int32 paramater, vaule:%d", value);

	return 1;
}

static int node_exit(bus_object_t *obj, int argc,
		             struct blob_attr_s **args,
                     void *out, int *out_len)
{
    bus_t *bus = obj->bus;
    Node *node = bus->opaque;

    *out_len = 0;
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
                             void *out, int *out_len)
{
    char buffer[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int db_bussiness, db_switch, db_level;

    db_bussiness = blob_get_uint32(args[0]);
    db_switch    = blob_get_uint32(args[1]);
    db_level     = blob_get_uint32(args[2]);

    debugger_set_business(debugger_gp, db_bussiness, db_switch, db_level);
    dbg_str(DBG_VIP, "set debug, bussiness=%d, switch=%d, level=%d", 
            db_bussiness, db_switch, db_level);

    memcpy(out, buffer, sizeof(buffer));
    *out_len = sizeof(buffer);

	return 1;
}

static const struct blob_policy_s write_file_policy[] = {
	[0] = { .name = "filename",  .type = BLOB_TYPE_STRING }, 
    [1] = { .name = "offset",    .type = BLOB_TYPE_UINT32 }, 
	[2] = { .name = "buffer",    .type = BLOB_TYPE_BUFFER }, 
    [3] = { .name = "crc32",     .type = BLOB_TYPE_UINT32 }, 
};

static int node_write_file(bus_object_t *obj, int argc, 
		      		       struct blob_attr_s **args, 
                           void *out, int *out_len)
{
    allocator_t *allocator;
    char *filename;
    uint8_t *buffer;
    uint32_t len, crc32, offset;
    bus_t *bus;
    File *file = NULL;
    int ret;

    TRY {
        THROW_IF(obj->bus == NULL, -1);
        bus = obj->bus;
        allocator = bus->allocator;

        filename = blob_get_string(args[0]);
        offset = blob_get_uint32(args[1]);
        len = blob_get_buffer(args[2], &buffer);
        crc32 = blob_get_uint32(args[3]);
        dbg_str(DBG_VIP, "file name:%s, data offset:%d, len:%d, crc32:%x", filename, offset, len, crc32);
        
        /* 如果写的offset 为0 需要先判断一下文件是否存在， 如果存在
         * 需要先删除，因为文件是分片追加的，刚开始写的时候不清零
         *会有问题。
         **/
        if(fs_is_exist(filename) && offset == 0) {
            EXEC(fs_rmfile(filename));
        } else if(!fs_is_exist(filename)) {
            EXEC(fs_mkfile(filename, 0777));
        }

        file = object_new(allocator, "File", NULL);
        EXEC(file->open(file, filename, "a+"));
        EXEC(file->seek(file, offset, SEEK_SET));
        EXEC(file->write(file, buffer, len));
        EXEC(file->close(file));
    } CATCH (ret) {} FINALLY {
        *out_len = 0;
        object_destroy(file);
    }

	return ret;
}

static const struct blob_policy_s read_file_policy[] = {
	[0] = { .name = "filename",  .type = BLOB_TYPE_STRING }, 
    [1] = { .name = "offset",    .type = BLOB_TYPE_UINT32 }, 
    [2] = { .name = "length",    .type = BLOB_TYPE_UINT32 }, 
};

static int node_read_file(bus_object_t *obj, int argc, 
		      		      struct blob_attr_s **args, 
                          void *out, int *out_len)
{
    char *filename;
    uint32_t len, offset;

    filename = blob_get_string(args[0]);
    offset = blob_get_uint32(args[1]);
    len = blob_get_uint32(args[2]);

    dbg_str(DBG_VIP, "file name:%s, data len:%d, offset:%d", filename, len, offset);

	return 1;
}

static const struct blob_policy_s list_policy[] = {
	[0] = { .name = "path",  .type = BLOB_TYPE_STRING }, 
};

static int node_list(bus_object_t *obj, int argc, 
		      		 struct blob_attr_s **args, 
                     void *out, int *out_len)
{
    bus_t *bus;
    char *path;
    uint32_t len, offset;
    Vector *list = NULL;
    allocator_t *allocator;
    int ret, count;

    TRY {
        path = blob_get_string(args[0]);
        THROW_IF(!fs_is_exist(path), -1);
        bus = obj->bus;
        allocator = bus->allocator;
        dbg_str(DBG_VIP, "node_list path:%s", path);

        list = object_new(allocator, "Vector", NULL);
        count = fs_tree(path, list, -1);
        THROW_IF(count < 0, -1);

        fs_print_file_info_list(list);
    } CATCH (ret) {} FINALLY {
        *out_len = 0;
        object_destroy(list);
    }

	return ret;
}

static const struct bus_method node_service_methods[] = {
	BUS_METHOD_WITHOUT_ARG("exit", node_exit, NULL),
    BUS_METHOD("test", node_test, test_policy),
    BUS_METHOD("set_loglevel", node_set_loglevel, set_loglevel_policy),
    BUS_METHOD("write_file", node_write_file, write_file_policy),
    BUS_METHOD("list", node_list, list_policy),
    BUS_METHOD("read_file", node_read_file, read_file_policy),
};

bus_object_t node_object = {
	.id        = (char *)"node",
    .cname     = (char *)"node_service", 
	.methods   = (struct bus_method *)node_service_methods,
	.n_methods = ARRAY_SIZE(node_service_methods),
};
