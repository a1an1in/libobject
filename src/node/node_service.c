#include <unistd.h>
#include <libobject/net/bus/bus.h>
#include <libobject/net/bus/busd.h>
#include <libobject/core/io/File.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/core/utils/string.h>
#include <libobject/core/utils/byteorder.h>
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
    node->node_exit_flag = 1;
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
        dbg_str(DBG_VIP, "node_read file name:%s, data len:%d, offset:%d", filename, len, offset);

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

        list = object_new(allocator, "Vector", NULL);
        list->set(list, "/Vector/value_type", &value_type);
        list->set(list, "/Vector/trustee_flag", &trustee_flag);
        list->set(list, "/Vector/class_name", "FS_File_Info");

        if (fs_is_directory(path) == 1) {
            count = fs_tree(path, list, -1);
        } else {
            count = fs_list(path, list);
        }
        THROW_IF(count < 0, -1);
        dbg_str(DBG_VIP, "node_list path:%s, count:%d", path, count);
        // dbg_str(DBG_DETAIL, "node_list json:%s", list->to_json(list));
        len = strlen(list->to_json(list));
        memcpy(out, list->to_json(list), len);
        *out_len = len;
        // fs_print_file_info_list(list);
    } CATCH (ret) {*out_len = 0;} FINALLY {
        object_destroy(list);
    }

	return ret;
}

static const struct blob_policy_s call_fshell_policy[] = {
	[0] = { .name = "command",  .type = BLOB_TYPE_STRING }, 
};

static int node_call_fshell(bus_object_t *obj, int argc, 
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
        node = bus->opaque;
        str = node->str;
        shell = node->shell;
        shell->opaque = bus;

        dbg_str(DBG_INFO, "node_call_fshell command:%s", command);
        str->reset(str);
        str->assign(str, command);
        THROW(shell->run_func(shell, str));
    } CATCH (ret) {} FINALLY {
        *out_len = 0;
    }

	return ret;
}

static const struct blob_policy_s call_cmd_policy[] = {
	[0] = { .name = "command",  .type = BLOB_TYPE_STRING }, 
};

static int popen_work_callback(void *task)
{
    work_task_t *t = task;
    Worker *worker = t->opaque;
    bus_object_t *obj = worker->opaque;
    bus_t *bus = obj->bus;
    // Map *map = bus->req_map;
    int ret, src_fd;

    TRY {
        src_fd = obj->src_fd;
        if (t->buf_len <= 0) {
            ret = t->buf_len;
            EXEC(bus_reply_forward_invoke(bus, obj->id, "call_cmd", BUS_RET_SUC, t->buf, 0, src_fd));
            dbg_str(DBG_DETAIL, "popen_work_callback, file addr:%p", worker->opaque);
            pclose(t->cache);
            object_destroy(worker);
            THROW(ret);
        }

        dbg_str(DBG_VIP, "popen_work_callback:%s, len:%d", t->buf, t->buf_len);
        EXEC(bus_reply_forward_invoke(bus, obj->id, "call_cmd", BUS_RET_PHASED_SUC, t->buf, t->buf_len, src_fd));
        // sprintf(t->name, "task@%d", src_fd);
        // map->add(map, t->name, t);
    } CATCH (ret) {}

    return ret;
}

static int popen_ev_callback(int fd, short event, void *arg)
{
    Worker *worker = (Worker *)arg;
    work_task_t *task = worker->task;
    int len;

    len = read(fd, task->buf, WORKER_TASK_MAX_BUF_LEN);
    if (len <= 0) {
        dbg_str(DBG_DETAIL, "popen_ev_callback, fd:%d, len=%d", fd, len);
    }

    if (worker->work_callback != NULL) {
        task->buf_len = len;
        task->fd = fd;
        task->opaque = worker;
        worker->work_callback(task);
    }

    return 0;
}

static int node_call_cmd(bus_object_t *obj, int argc, 
                        struct blob_attr_s **args, 
                        void *out, int *out_len)
{
    char *command;
    FILE *f = NULL;
    bus_t *bus = obj->bus;
    Map *map = bus->req_map;
    allocator_t *allocator;
    Worker *worker;
    work_task_t *task;
    int ret, len, fd;

    TRY {
        command = blob_get_string(args[0]);
        dbg_str(DBG_VIP, "node call_cmd command:%s", command);
        THROW_IF(command == NULL, -1);
        if (command[0] == '"') {
            len = strlen(command);
            command[len - 1] = '\0';
            command = &command[1];
        }

        f = popen(command, "r");
        fd = fileno(f);
        allocator = bus->allocator;

        worker = io_worker(allocator, fd, NULL, NULL, popen_ev_callback, 
                           popen_work_callback, obj); //shell opaque是bus
        task = worker->task;
        task->cache = f;  // popen由worker负责释放
        task->socket = obj->src_fd;  // 传入src_fd用于异步回复。

        sprintf(worker->name, "%d@call_cmd", obj->src_fd);
        EXEC(map->add(map, worker->name, worker));
        THROW(BUS_RET_PHASED_SUC);
    } CATCH (ret) {} FINALLY {
        *out_len = 0;
    }

	return ret;
}

static const struct blob_policy_s call_fshell_object_method_policy[] = {
	[0] = { .name = "command",  .type = BLOB_TYPE_STRING }, 
};

static int node_call_fshell_object_method(bus_object_t *obj, int argc, 
                                          struct blob_attr_s **args, 
                                          void *out, int *out_len)
{
    bus_t *bus;
    char *command;
    FShell *shell;
    String *str;
    Node *node;
    int ret;

    TRY {
        command = blob_get_string(args[0]);
        bus = obj->bus;
        node = bus->opaque;
        shell = node->shell;
        str = node->str;

        dbg_str(DBG_INFO, "node_call_fshell_object_method command:%s", command);
        str->reset(str);
        str->assign(str, command);
        THROW(shell->run_object_func(shell, str));
    } CATCH (ret) {} FINALLY {
        *out_len = 0;
    }

	return ret;
}

/* 因为alloc要返回地址， 如果在fshell里实现， 很难把地址传出来，
 * 需要在node_cli 指定存储地址的空间， 这样使用会更复杂。 */
static const struct blob_policy_s malloc_policy[] = { 
    [0] = { .name = "target_type",  .type = BLOB_TYPE_UINT32 }, 
    [1] = { .name = "value_type",   .type = BLOB_TYPE_UINT32 },
    [2] = { .name = "class_name",   .type = BLOB_TYPE_STRING },
    [3] = { .name = "name",         .type = BLOB_TYPE_STRING },
    [4] = { .name = "size",         .type = BLOB_TYPE_UINT32 }, 
};
static int node_malloc(bus_object_t *obj, int argc, 
                       struct blob_attr_s **args, 
                       void *out, int *out_len)
{
    bus_t *bus;
    allocator_t *allocator;
    char *name, *class_name, *addr;
    target_type_t type;
    uint32_t value_type;
    Node *node;
    FShell *shell;
    Map *map;
    fsh_malloc_variable_info_t *info;
    int ret, size;

    TRY {
        type = blob_get_uint32(args[0]);
        value_type = blob_get_uint32(args[1]);
        class_name = blob_get_string(args[2]);
        name = blob_get_string(args[3]);
        size = blob_get_uint32(args[4]);
        bus = obj->bus;
        node = bus->opaque;
        shell = node->shell;
        map = shell->variable_map;
        allocator = bus->allocator;

        EXEC(fsh_variable_info_alloc(allocator, value_type, class_name, size, name, &info));
        addr = info->addr;
        map->add(map, info->name, info);
        
        dbg_str(DBG_VIP, "node_malloc, name:%s, class_name:%s, size:%d, addr:%p", name, class_name, size, addr);
        *out_len = sizeof(void *);
        addr = byteorder_cpu_to_be64(&addr);
        memcpy(out, &addr, sizeof(void *));
    } CATCH (ret) {*out_len = 0;} FINALLY { }

	return ret;
}

static const struct blob_policy_s mfree_policy[] = {
    [0] = { .name = "target_type",  .type = BLOB_TYPE_UINT32 },
    [1] = { .name = "name",         .type = BLOB_TYPE_STRING },
};
static int node_mfree(bus_object_t *obj, int argc, 
                      struct blob_attr_s **args, 
                      void *out, int *out_len)
{
    bus_t *bus;
    allocator_t *allocator;
    Node *node;
    FShell *shell;
    char *name, *search_addr = NULL;
    Map *map;
    fsh_malloc_variable_info_t *info = NULL;
    target_type_t type;
    int ret, size;

    TRY {
        type = blob_get_uint32(args[0]);
        name = blob_get_string(args[1]);
        bus = obj->bus;
        node = bus->opaque;
        shell = node->shell;
        map = shell->variable_map;
        allocator = bus->allocator;
        dbg_str(DBG_DETAIL, "node_mfree, name:%s", name);

        THROW_IF(name == NULL, -1);
        map->remove(map, name, &info);
        THROW_IF(info == NULL, -1);
        EXEC(fsh_variable_info_free(allocator, info));
    } CATCH (ret) { } FINALLY { *out_len = 0; }

	return ret;
}

static const struct blob_policy_s mget_policy[] = {
    [0] = { .name = "target_type", .type = BLOB_TYPE_UINT32 }, 
    [1] = { .name = "addr",        .type = BLOB_TYPE_UINT64 },
    [2] = { .name = "offset",      .type = BLOB_TYPE_UINT32 },
    [3] = { .name = "capacity",    .type = BLOB_TYPE_UINT32 },
    [4] = { .name = "len",         .type = BLOB_TYPE_UINT32 },
};
static int node_mget(bus_object_t *obj, int argc, 
                     struct blob_attr_s **args, 
                     void *out, int *out_len)
{
    bus_t *bus;
    allocator_t *allocator;
    char *addr;
    target_type_t type;
    Node *node;
    int ret, size, offset, capacity, len;

    TRY {
        type = blob_get_uint32(args[0]);
        addr = blob_get_uint64(args[1]);
        offset = blob_get_uint32(args[2]);
        capacity = blob_get_uint32(args[3]);
        len = blob_get_uint32(args[4]);
        bus = obj->bus;
        node = bus->opaque;
        allocator = bus->allocator;
        dbg_str(DBG_VIP, "node_mget, type:%d addr:%p, offset:%d, capacity:%d", type, addr, offset, capacity);

        memcpy(out, addr, len);
        *out_len = len;
    } CATCH (ret) {*out_len = 0;} FINALLY { }

	return ret;
}

static const struct blob_policy_s mset_policy[] = {
    [0] = { .name = "target_type", .type = BLOB_TYPE_UINT32 }, 
    [1] = { .name = "addr",        .type = BLOB_TYPE_UINT64 },
    [2] = { .name = "offset",      .type = BLOB_TYPE_UINT32 },
    [3] = { .name = "capacity",    .type = BLOB_TYPE_UINT32 },
    [4] = { .name = "value",       .type = BLOB_TYPE_BUFFER }, 
};
static int node_mset(bus_object_t *obj, int argc, 
                     struct blob_attr_s **args, 
                     void *out, int *out_len)
{
    bus_t *bus;
    allocator_t *allocator;
    char *addr;
    target_type_t type;
    uint8_t *buffer;
    Node *node;
    int ret, size, offset, len, capacity;

    TRY {
        type = blob_get_uint32(args[0]);
        addr = blob_get_uint64(args[1]);
        offset = blob_get_uint32(args[2]);
        capacity = blob_get_uint32(args[3]);
        len = blob_get_buffer(args[4], &buffer);
    
        bus = obj->bus;
        node = bus->opaque;
        allocator = bus->allocator;
        THROW_IF(len > capacity || addr == NULL, -1);

        dbg_str(DBG_VIP, "node_mset, type:%d addr:%p, offset:%d, capacity:%d, len:%d", 
                type, addr, offset, capacity, len);
        dbg_buf(DBG_VIP, "node mset receive buffer:", buffer, len);
        memcpy(addr + offset, buffer, len);
    } CATCH (ret) { } FINALLY { *out_len = 0; }

	return ret;
}

/* 注意：只能使用指针的地址获取指针。 没有复用mget是因为使用它获取多级指针
 * 对应的地址需要多一个node大小端类型的接口，而且单独一个接口用来取多级
 * 指针对应的地址也是必要的，需要合并简化操作。 */
static const struct blob_policy_s mget_pointer_policy[] = {
    [0] = { .name = "target_type", .type = BLOB_TYPE_UINT32 }, 
    [1] = { .name = "addr",        .type = BLOB_TYPE_UINT64 },
};
static int node_maddress(bus_object_t *obj, int argc, 
                             struct blob_attr_s **args, 
                             void *out, int *out_len)
{
    bus_t *bus;
    allocator_t *allocator;
    char **addr;
    target_type_t type;
    uint8_t *buffer;
    Node *node;
    void *value;
    int ret, size, offset;

    TRY {
        type = blob_get_uint32(args[0]);
        addr = blob_get_uint64(args[1]);
    
        bus = obj->bus;
        node = bus->opaque;
        allocator = bus->allocator;
        THROW_IF(addr == NULL, -1);

        addr = byteorder_be64_to_cpu(&addr);
        value = *addr;
        value = byteorder_cpu_to_be64(&value);
        memcpy(out, &value, sizeof(void *));

        *out_len = sizeof(void *);
    } CATCH (ret) { *out_len = 0; } FINALLY { }

	return ret;
}

static const struct blob_policy_s mget_addr_policy[] = {
    [0] = { .name = "target_type",  .type = BLOB_TYPE_UINT32 },
    [1] = { .name = "name",         .type = BLOB_TYPE_STRING },
};
static int node_mget_addr(bus_object_t *obj, int argc, 
                          struct blob_attr_s **args, 
                          void *out, int *out_len)
{
    bus_t *bus;
    allocator_t *allocator;
    Node *node;
    FShell *shell;
    char *name, *addr;
    Map *map;
    fsh_malloc_variable_info_t *info = NULL;
    target_type_t type;
    int ret, size;

    TRY {
        type = blob_get_uint32(args[0]);
        name = blob_get_string(args[1]);
        bus = obj->bus;
        node = bus->opaque;
        shell = node->shell;
        map = shell->variable_map;
        allocator = bus->allocator;
        dbg_str(DBG_DETAIL, "node mget addr, name:%s", name);

        THROW_IF(name == NULL, -1);
        map->search(map, name, &info);
        THROW_IF(info == NULL, -1);
        EXEC(fsh_variable_info_free(allocator, info));

        addr = info->addr;
        *out_len = sizeof(void *);
        addr = byteorder_cpu_to_be64(&addr);
        memcpy(out, &addr, sizeof(void *));
    } CATCH (ret) { *out_len = 0; } FINALLY {  }

	return ret;
}


static int node_abort_cmd(bus_object_t *obj, int argc, 
                          struct blob_attr_s **args, 
                          void *out, int *out_len)
{
    bus_t *bus;
    Map *map;
    Worker *worker;
    work_task_t *task;
    char key[MAX_WORKER_NAME_LEN];
    int ret;

    TRY {
        // name = blob_get_string(args[0]);
        bus = obj->bus;
        map = bus->req_map;
        sprintf(key, "%d@call_cmd", obj->src_fd);
        ret = map->search(map, key, (void **)&worker);
        THROW_IF(ret <= 0, -1);

        dbg_str(DBG_FATAL, "node_abort_cmd find src fd:%d", obj->src_fd);
        task = worker->task;
        pclose(task->cache);
        object_destroy(worker);
    } CATCH (ret) {} FINALLY {
        *out_len = 0;
    }

	return ret;
}

static const struct bus_method node_service_methods[] = {
    BUS_METHOD_WITHOUT_ARG("exit", node_exit, NULL),
    BUS_METHOD("test", node_test, test_policy),
    BUS_METHOD("set_loglevel", node_set_loglevel, set_loglevel_policy),
    BUS_METHOD("flist", node_list, list_policy),
    BUS_METHOD("fwrite", node_write_file, write_file_policy),
    BUS_METHOD("fread", node_read_file, read_file_policy),
    BUS_METHOD("call_fshell", node_call_fshell, call_fshell_policy),
    BUS_METHOD("call_fshell_object_method", node_call_fshell_object_method, call_fshell_object_method_policy),
    BUS_METHOD("call_cmd", node_call_cmd, call_cmd_policy),
    BUS_METHOD("malloc", node_malloc, malloc_policy),
    BUS_METHOD("mfree", node_mfree, mfree_policy),
    BUS_METHOD("mget", node_mget, mget_policy),
    BUS_METHOD("mget_addr", node_mget_addr, mget_addr_policy),
    BUS_METHOD("mset", node_mset, mset_policy),
    BUS_METHOD("maddress", node_maddress, mget_pointer_policy),
    BUS_METHOD_WITHOUT_ARG("abort_cmd", node_abort_cmd, NULL),
};

bus_object_t node_object = {
    .cname     = (char *)"node_service", 
    .methods   = (struct bus_method *)node_service_methods,
    .n_methods = ARRAY_SIZE(node_service_methods),
};
