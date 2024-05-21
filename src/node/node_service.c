#include <unistd.h>
#include <libobject/net/bus/bus.h>
#include <libobject/net/bus/busd.h>
#include <libobject/core/io/File.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/core/utils/string.h>
#include <libobject/node/Node.h>
#include <libobject/scripts/fshell/api.h>

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
        if (fs_is_exist(filename) && offset == 0) {
            EXEC(fs_rmfile(filename));
        } else if (!fs_is_exist(filename)) {
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
    File *file = NULL;
    allocator_t *allocator;
    bus_t *bus;
    int ret;

    TRY {
        THROW_IF(obj->bus == NULL, -1);
        bus = obj->bus;
        allocator = bus->allocator;
        
        filename = blob_get_string(args[0]);
        offset = blob_get_uint32(args[1]);
        len = blob_get_uint32(args[2]);
        dbg_str(DBG_VIP, "file name:%s, data len:%d, offset:%d", filename, len, offset);

        THROW_IF(!fs_is_exist(filename), -1);

        file = object_new(allocator, "File", NULL);
        EXEC(file->open(file, filename, "a+"));
        EXEC(file->seek(file, offset, SEEK_SET));
        EXEC(file->read(file, out, len));
        EXEC(file->close(file));

        *out_len = len;
    } CATCH (ret) {
        *out_len = 0;
    } FINALLY {
        object_destroy(file);
    }

	return ret;
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
    int value_type = VALUE_TYPE_STRUCT_POINTER;
    uint8_t trustee_flag = 1;
    uint32_t len;
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
        list->set(list, "/Vector/value_type", &value_type);
        list->set(list, "/Vector/trustee_flag", &trustee_flag);
        list->set(list, "/Vector/value_to_json_callback", fs_file_info_struct_custom_to_json);
        list->set(list, "/Vector/value_free_callback", fs_file_info_struct_custom_free);
        count = fs_tree(path, list, -1);
        THROW_IF(count < 0, -1);
        dbg_str(DBG_VIP, "node_list json:%s", list->to_json(list));
        len = strlen(list->to_json(list));
        memcpy(out, list->to_json(list), len);
        *out_len = len;
        // fs_print_file_info_list(list);
    } CATCH (ret) {*out_len = 0;} FINALLY {
        object_destroy(list);
    }

	return ret;
}

/*
 * node 只会使用一个fshell， 所有可以把shell保存在bus中。
 */
static int node_open_fshell(bus_object_t *obj, int argc, 
                            struct blob_attr_s **args, 
                            void *out, int *out_len)
{
    bus_t *bus;
    void *shell = NULL;
    allocator_t *allocator;
    int ret;

    TRY {
        bus = obj->bus;
        allocator = bus->allocator;

        dbg_str(DBG_VIP, "node_open_fshell in");
#if (defined(WINDOWS_USER_MODE))
        shell = object_new(allocator, "WindowsFShell", NULL);    
#else  
        shell = object_new(allocator, "UnixFShell", NULL);
#endif

        bus->shell = shell;
    } CATCH (ret) {} FINALLY {
        *out_len = 0;
    }

	return ret;
}

static const struct blob_policy_s exec_fshell_policy[] = {
	[0] = { .name = "command",  .type = BLOB_TYPE_STRING }, 
};

static int node_exec_fshell(bus_object_t *obj, int argc, 
                            struct blob_attr_s **args, 
                            void *out, int *out_len)
{
    bus_t *bus;
    char *command;
    allocator_t *allocator;
    FShell *shell;
    String *str;
    Node *node;
    int ret;

    TRY {
        command = blob_get_string(args[0]);
        bus = obj->bus;
        allocator = bus->allocator;
        shell = bus->shell;
        node = bus->opaque;
        str = node->str;

        dbg_str(DBG_VIP, "node_exec_fshell command:%s", command);
        str->reset(str);
        str->assign(str, command);
        THROW(shell->run_func(shell, str));
    } CATCH (ret) {} FINALLY {
        *out_len = 0;
    }

	return ret;
}

static int node_close_fshell(bus_object_t *obj, int argc, 
                             struct blob_attr_s **args, 
                             void *out, int *out_len)
{
    bus_t *bus;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "node_close_fshell in");
        bus = obj->bus;
        object_destroy(bus->shell);
        bus->shell = NULL;
    } CATCH (ret) {*out_len = 0;} FINALLY { }

	return ret;
}

/* 因为alloc要返回地址， 如果在fshell里实现， 很难把地址传出来，
 * 需要在node_cli 指定存储地址的空间， 这样使用会更复杂。 */
static const struct blob_policy_s alloc_policy[] = { 
    [0] = { .name = "size",  .type = BLOB_TYPE_UINT32 }, 
    [1] = { .name = "name",  .type = BLOB_TYPE_STRING },
};
static int node_alloc(bus_object_t *obj, int argc, 
                      struct blob_attr_s **args, 
                      void *out, int *out_len)
{
    bus_t *bus;
    allocator_t *allocator;
    char *name;
    void *addr;
    int ret, size;

    TRY {
        size = blob_get_uint32(args[0]);
        name = blob_get_string(args[1]);
        bus = obj->bus;
        allocator = bus->allocator;
        addr = allocator_mem_alloc(allocator, size);

        *out_len = 2 * sizeof(void *) + 3;
        snprintf(out, *out_len, "0x%p", addr);
        dbg_str(DBG_VIP, "node_alloc, name:%s size:%d addr:%s", name, size, out);
    } CATCH (ret) {*out_len = 0;} FINALLY { }

	return ret;
}

/* node bus 参数都是用字符串传入的， 所以addr 用字符传要方便一些。*/
static const struct blob_policy_s free_policy[] = { 
    [0] = { .name = "addr",  .type = BLOB_TYPE_STRING }, 
    [1] = { .name = "name",  .type = BLOB_TYPE_STRING },
};
static int node_free(bus_object_t *obj, int argc, 
                     struct blob_attr_s **args, 
                     void *out, int *out_len)
{
    bus_t *bus;
    allocator_t *allocator;
    char *name;
    char *addr;
    long long a;
    int ret, size;

    TRY {
        addr = blob_get_string(args[0]);
        name = blob_get_string(args[1]);
        bus = obj->bus;
        allocator = bus->allocator;
        a = str_hex_to_integer(addr);

        dbg_str(DBG_VIP, "node_free, name:%s addr:%s, digital integer:%llx", name, addr, a);
        allocator_mem_free(allocator, a);
    } CATCH (ret) {*out_len = 0;} FINALLY { }

	return ret;
}

static const struct bus_method node_service_methods[] = {
	BUS_METHOD_WITHOUT_ARG("exit", node_exit, NULL),
    BUS_METHOD("test", node_test, test_policy),
    BUS_METHOD("set_loglevel", node_set_loglevel, set_loglevel_policy),
    BUS_METHOD("list", node_list, list_policy),
    BUS_METHOD("write_file", node_write_file, write_file_policy),
    BUS_METHOD("read_file", node_read_file, read_file_policy),
    BUS_METHOD_WITHOUT_ARG("open_fshell", node_open_fshell, NULL),
    BUS_METHOD_WITHOUT_ARG("close_fshell", node_close_fshell, NULL),
    BUS_METHOD("exec_fshell", node_exec_fshell, exec_fshell_policy),
    BUS_METHOD("alloc", node_alloc, alloc_policy),
    BUS_METHOD("free", node_free, free_policy),
};

bus_object_t node_object = {
	.id        = (char *)"node",
    .cname     = (char *)"node_service", 
	.methods   = (struct bus_method *)node_service_methods,
	.n_methods = ARRAY_SIZE(node_service_methods),
};
