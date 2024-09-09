/**
 * @file node.c
 * @Synopsis
 *  node的设计是为了抽象通信节点，主要职责是：
 *  1.node通过node deamon使任意两个node能够正常通信， 不需要再设计通信协议， 
 *    可以相互传输json，然后可以很方便的反序列化成对应的结构体。
 *  2.支持node文件互传。
 *  3.支持调用远端node的常规函数或者对象方法。
 *  4.支持版本检测和自动升级。
 *  5.支持节点状态检测。
 *  6.stub和attancher动态调试。
 *  7.动态部署服务。
 *  8.分布式应用基础。
 *  node相当于构建了两个端点的基础设置，不用再重复关心这些节点涉及的同类问题。
 * @author alan lin
 * @version 
 * @date 2024-02-18
 */
#include <libobject/argument/Application.h>
#include <libobject/core/io/file_system_api.h>
#include <libobject/core/io/File.h>
#include <libobject/core/utils/string.h>
#include <libobject/core/utils/byteorder.h>
#include <libobject/encoding/Digest.h>
#include <libobject/node/Node.h>

static int __construct(Node *node, char *init_str)
{
    allocator_t *allocator = node->parent.allocator;
    Map *map;
    int trustee_flag = 1;
    int value_type = VALUE_TYPE_STRUCT_POINTER;

    node->str = object_new(allocator, "String", NULL);
    node->busd = NULL;
    node->bus = NULL;
    
    return 0;
}

static int __deconstruct(Node *node)
{
    object_destroy(node->str);
    bus_destroy(node->bus);

    //需要等待客户端关闭连接， 然后服务器也处理关闭事务， 不然
    //如果server也同时销毁， 有可能会同时操作worker链表，导致异常。
    //因为使用的event base, 所以没有给链表加锁，但是销毁server和对
    //客户端关闭处理是在俩个不通线程处理，所以需要等待一下， 这个不算
    //是问题， 是异步设计需要注意的一个地方。
    usleep(1000); 
    busd_destroy(node->busd);

    return 0;
}

static int __init(Node *node)
{
    allocator_t *allocator = node->parent.allocator;
    Application *app;
    bus_t *bus = NULL;
    busd_t *busd = NULL;
    Digest *digest;
    int i, ret;

    TRY {
        THROW_IF(node->host == NULL || node->service == NULL, -1);

        dbg_str(DBG_VIP, "node host:%s, service:%s", node->host, node->service);
        if (node->run_bus_deamon_flag == 1) {
            busd = busd_create(allocator, node->host,
                               node->service, SERVER_TYPE_INET_TCP);
            THROW_IF(busd == NULL, -1);
            node->busd = busd;
        }

        bus = bus_create(allocator, node->host,
                         node->service, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);
        node->bus = bus;
        bus->opaque = node;

        if (node->disable_node_service_flag != 1) {
            if (node->node_id != NULL) {
                memcpy(&node_object.id, node->node_id, strlen(node->node_id));
            } else {
                // if (node->disable_node_digest_id_flag != 1) {
                //     digest = object_new(allocator, "Digest_Sha1", NULL);
                //     digest->init(digest);
                //     digest->update(digest, "test", strlen("test"));
                //     digest->final(digest, &node_object.id, 20);
                // }
            }
            bus_add_object(bus, &node_object);
            app = get_global_application();
            node->shell = app->fshell;
        }
    } CATCH (ret) {}

    return ret;
}

static int __loop(Node *node)
{
    do { sleep(1); } while (node->node_exit_flag != 1);
    dbg_str(DBG_VIP, "node loop out");
    return 0;
}

/* 
 * 这个地方约束了call对客户只支持node_object(默认调用的就是这个), 原因有下：
 * 1.如果有多个不同的object，客户端不知道客户使用的是哪个类型的object， 如果去
 *   bus查询，会比较麻烦， 还要把json转换成struct， 目前不支持这种转换， 如果
 *   要转Oject倒是容易，但是bus设计是在设计Object之前。 
 * 2.如果要提供多种不同的bus object class, 有很大开发工作量，而且node 提供
 *   fshell的接口，就不需要做额外的开发了， 直接就可以调用函数， 不用再对各种不同
 *   应用写不同bus接口了。
 * 3.如果node之间调用不需要返回数据就不需要看开发新的接口，可以之间使用call_bus.
 */
static int __call_bus(Node *node, char *code, void *out, uint32_t *out_len)
{
    allocator_t *allocator = node->parent.allocator;
    char *method_name = NULL;
    char *node_id = NULL, *tmp, *p;
    bus_method_args_t *args = NULL;
    String *str = node->str;
    int ret, argc, i, count;

    TRY {
        dbg_str(DBG_VIP, "call_bus in, code:%s", code);
        EXEC(str->reset(str));
        str->assign(str, code);
        count = str->split(str, "[,@ \t\n();]", -1);
        THROW_IF(count < 2, -1);

        node_id = str->get_splited_cstr(str, 0);
        method_name = str->get_splited_cstr(str, 1);
        EXEC(node_find_method_argument_template(&node_object, allocator, method_name, &args, &argc));
        THROW_IF(count - 2 != argc, -1); /* 除去node 和 方法名 */
        for (i = 0; i < argc; i++) {
            tmp = str->get_splited_cstr(str, 2 + i);
            if (args[i].type == ARG_TYPE_UINT32) {
                args[i].value = atoi(str->get_splited_cstr(str, 2 + i));
            } else if (args[i].type == ARG_TYPE_UINT64 && tmp[0] == '0' && (tmp[1] == 'x' || tmp[1] == 'X')) {
                args[i].value = str_hex_to_integer(tmp);
                dbg_str(DBG_INFO, "call_bus ARG_TYPE_UINT64:%p, hex string:%s", args[i].value, tmp);
            } else if (args[i].type == ARG_TYPE_BUFFER) {
                dbg_str(DBG_VIP, "call_bus ARG_TYPE_BUFFER value:%s", tmp);
                p = strchr(tmp, ':');
                THROW_IF(p == NULL, -1);
                *p = '\0';
                args[i].value = str_hex_to_integer(tmp);
                args[i].len = atoi(p + 1);
            } else {
                args[i].value = str->get_splited_cstr(str, 2 + i);
            }
        }
        EXEC(bus_invoke_sync(node->bus, node_id, method_name, argc, args, out, out_len)); 
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "call_bus argc:%d, count:%d", argc, count);
    } FINALLY {
        node_free_argument_template(allocator, args, argc);
    }

    return ret;
}

/*
 * 注意：
 * 1.这个不能复用call_bus, 因为execute不想把命令的参数也解析出来。如果加标记判断
 * 什么时候解析，会把call_bus搞复杂了。
 * 2.call fsh不知道哪些函数需要回传什么特定的数据， 所以设计只能回传return value。
 * 如果需要调用的同时回传数据，请可以使用call_bus或者结合使用call_fsh和mget。
 */
static int __call_fsh(Node *node, const char *fmt, ...)
{
    bus_t *bus;
    va_list ap;
    String *str = node->str;
    char *node_id, *command;
    char code[1024] = {0};
    bus_method_args_t args[1] = {
        [0] = {ARG_TYPE_STRING, "command", NULL}, 
    };
    int ret, count;

    TRY {
        va_start(ap, fmt);
        vsnprintf(code, MAX_DBG_STR_LEN, fmt, ap);
        va_end(ap);

        EXEC(str->reset(str));
        str->assign(str, code);
        count = str->split(str, "[@]", -1);
        THROW_IF(count != 2, -1);
        bus = node->bus;

        node_id = str->get_splited_cstr(str, 0);
        command = str->get_splited_cstr(str, 1);
        args[0].value = command;
        EXEC(bus_invoke_sync(bus, node_id, "call_fshell", ARRAY_SIZE(args), args, NULL, NULL));
    } CATCH (ret) {}

    return ret;
}

static int __call_fsh_object_method(Node *node, const char *fmt, ...)
{
    bus_t *bus;
    va_list ap;
    String *str = node->str;
    char *node_id, *command;
    char code[1024] = {0};
    bus_method_args_t args[1] = {
        [0] = {ARG_TYPE_STRING, "command", NULL}, 
    };
    int ret, count;

    TRY {
        va_start(ap, fmt);
        vsnprintf(code, MAX_DBG_STR_LEN, fmt, ap);
        va_end(ap);

        EXEC(str->reset(str));
        str->assign(str, code);
        count = str->split(str, "[@]", -1);
        THROW_IF(count != 2, -1);
        bus = node->bus;

        node_id = str->get_splited_cstr(str, 0);
        command = str->get_splited_cstr(str, 1);
        args[0].value = command;

        // object_call_method
        EXEC(bus_invoke_sync(bus, node_id, "call_fshell_object_method", ARRAY_SIZE(args), args, NULL, NULL));
    } CATCH (ret) {}

    return ret;
}

static int __write_file(Node *node, char *from, char *node_id, char *to)
{
    bus_t *bus;
    File *file = NULL;
    allocator_t *allocator;
	char buffer[1024] = {0}, file_name;
    uint32_t buffer_size = sizeof(buffer);
    uint32_t size, i, j, read_len = 0, offset = 0, count = 0;
    uint32_t file_name_len;
    Vector *list = NULL;
    fs_file_info_t *fs_file_info;
    char *relative_path;
    bus_method_args_t args[4] = {
        [0] = {ARG_TYPE_STRING, "filename", to}, 
        [1] = {ARG_TYPE_UINT32, "offset", offset}, 
        [2] = {ARG_TYPE_BUFFER, "buffer", buffer, 0}, 
        [3] = {ARG_TYPE_UINT32, "crc32", 0x123}, 
    };
    int ret;

    TRY {
        dbg_str(DBG_VIP, "node write in");
        THROW_IF(node == NULL || node_id == NULL, -1);
        THROW_IF(from == NULL || to == NULL, -1);
        THROW_IF(!fs_is_exist(from), -1);

        allocator = node->parent.allocator;
        bus = node->bus;
        if (fs_is_directory(from)) {
            list = object_new(allocator, "Vector", NULL);
            count = fs_tree(from, list, -1);
            THROW_IF(count < 0, -1);
            // fs_print_file_info_list(list);

            for (i = 0; i < count; i++) {
                EXEC(list->peek_at(list, i, &fs_file_info));
                THROW_IF(fs_file_info == NULL, -1);

                file_name_len = strlen(fs_file_info->file_name);
                if ((fs_file_info->file_name[file_name_len - 1] == '.')) continue;
                
                EXEC(fs_get_relative_path(fs_file_info->file_name, from, &relative_path));
                
                strcpy(buffer, to);
                strcat(buffer, relative_path);
                dbg_str(DBG_VIP, "dest file name:%s", buffer);
                EXEC(node->write_file(node, fs_file_info->file_name, node_id, buffer))
            }
        } else {
            dbg_str(DBG_VIP, "run at here, open file:%s", from);
            file = object_new(allocator, "File", NULL);
            EXEC(file->open(file, from, "r+"));
            size = fs_get_size(from);
            dbg_str(DBG_VIP, "run at here, size=%d, num:%d", size, size / buffer_size);
            for (j = 0; j <= (size / buffer_size); j++) {
                read_len = file->read(file, buffer, buffer_size);
                args[1].value = offset;
                args[2].len = read_len;
                dbg_str(DBG_VIP, "read %s, offset:%d, read size:%d, read len:%d", 
                        from, offset, buffer_size, read_len);
                EXEC(bus_invoke_sync(bus, "node", "write_file", ARRAY_SIZE(args), args, NULL, NULL));
                offset += read_len;
            }
            file->close(file);
        }
    } CATCH (ret) {} FINALLY {
        object_destroy(file);
        object_destroy(list);
    }

    return ret;
}

static int __read_file(Node *node, char *node_id, char *from, char *to)
{
    bus_t *bus;
    File *file = NULL;
    allocator_t *allocator;
	char buffer[BLOB_BUFFER_MAX_SIZE] = {0}, file_name;
    uint32_t read_size = 1024;
    uint32_t size, i, j, read_len = 0, offset = 0, count = 0;
    uint32_t file_name_len;
    Vector *list = NULL;
    fs_file_info_t *fs_file_info;
    char *relative_path;
    bus_method_args_t args[3] = {
        [0] = {ARG_TYPE_STRING, "filename", NULL}, 
        [1] = {ARG_TYPE_UINT32, "offset", offset}, 
        [2] = {ARG_TYPE_UINT32, "length", 0}, 
    };
    int ret;

    TRY {
        dbg_str(DBG_VIP, "node read in");
        THROW_IF(node == NULL || node_id == NULL, -1);
        THROW_IF(from == NULL || to == NULL, -1);

        allocator = node->parent.allocator;
        bus = node->bus;

        list = object_new(allocator, "Vector", NULL);
        EXEC(node->list(node, node_id, from, list));
        count = list->count(list);
        THROW_IF(count < 0, -1);
        file = object_new(allocator, "File", NULL);

        for (i = 0; i < count; i++) {
            offset = 0;
            EXEC(list->peek_at(list, i, &fs_file_info));
            THROW_IF(fs_file_info == NULL, -1);

            file_name_len = strlen(fs_file_info->file_name);
            if ((fs_file_info->file_name[file_name_len - 1] == '.')) continue;
            
            size = fs_file_info->st.st_size;
            dbg_str(DBG_VIP, "read index: %d, file name:%s, size:%d", i, fs_file_info->file_name, size);
            
            EXEC(fs_get_relative_path(fs_file_info->file_name, from, &relative_path));
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, to);
            strcat(buffer, relative_path);
            dbg_str(DBG_VIP, "dest file name:%s", buffer);

            if (!fs_is_exist(buffer)) {
                EXEC(fs_mkfile(buffer, 0777));
            }
            EXEC(file->open(file, buffer, "a+"));
            for (j = 0; j <= (size / read_size); j++) {
                args[0].value = fs_file_info->file_name;
                args[1].value = offset;
                args[2].value = (size - j * read_size) > read_size ? read_size : (size - j * read_size);
                
                EXEC(bus_invoke_sync(bus, "node", "read_file", ARRAY_SIZE(args), args, buffer, &args[2].value));
                dbg_str(DBG_VIP, "read %s, offset:%d, read size:%d, read len:%d", 
                        from, offset, read_size, args[2].value);
                
                EXEC(file->seek(file, offset, SEEK_SET));
                EXEC(file->write(file, buffer, (int)args[2].value));
                offset += (uint32_t)args[2].value;
            }
            
            EXEC(file->close(file));
            fs_file_info = NULL;
        }
    } CATCH (ret) {} FINALLY {
        object_destroy(file);
        object_destroy(list);
    }

    return ret;
}

static int __copy(Node *node, char *from, char *to)
{
    char *p1, *p2, *node_id;
    int read_flag = 0, write_flag = 0;
    int ret;

    TRY {
        if ((p1 = strchr(from, ':')) != NULL) {
            read_flag = 1;
            node_id = from;
            *p1 = '\0';
            from = p1 + 1;
        }

        if ((p2 = strchr(to, ':')) != NULL) {
            THROW_IF(read_flag == 1, -1);
            write_flag = 1;
            node_id = to;
            *p2 = '\0';
            to = p2 + 1;
        }
        THROW_IF(p1 == NULL && p2 == NULL, -1);
        THROW_IF(p1 != NULL && p2 != NULL, -1);
        dbg_str(DBG_VIP, "node copy, node:%s, from:%s, to:%s", node_id, from, to);

        if (read_flag == 1) {
            EXEC(node->read_file(node, node_id, from , to));
        } else if (write_flag == 1) {
            EXEC(node->write_file(node, from, node_id, to));
        } else {
            THROW(-1);
        }
    } CATCH (ret) {}

    return ret;
}

static int __list(Node *node, char *node_id, char *path, Vector *vector)
{
    int value_type = VALUE_TYPE_STRUCT_POINTER;
    uint8_t trustee_flag = 1;
    int read_flag = 0, write_flag = 0;
    char buffer[BLOB_BUFFER_MAX_SIZE];
    int len = sizeof(buffer);
    int ret;

    TRY {
        dbg_str(DBG_VIP, "node list node_id:%s, path:%s", node_id, path);

        THROW_IF(path == NULL, -1);
        snprintf(buffer, sizeof(buffer), "%s@list(%s)", node_id, path);
        EXEC(node->call_bus(node, buffer, buffer, &len));
        THROW_IF(len == 0, 0);
        if (len > 0) {
            buffer[len] = 0;
        }

        vector->set(vector, "/Vector/value_type", &value_type);
        vector->set(vector, "/Vector/trustee_flag", &trustee_flag);
        vector->set(vector, "/Vector/class_name", "FS_File_Info");
        vector->assign(vector, buffer);
    } CATCH (ret) {}

    return ret;
}

static int __malloc(Node *node, char *node_id, target_type_t type, int value_type, char *class_name, char *name, int size, void **addr)
{
    char cmd[1024] = {0};
    int ret, len = sizeof(addr);
    
    TRY {
        snprintf(cmd, 1024, "node@malloc(%d, %d, %s, %s, %d)", type, value_type, 
                 class_name == NULL ? "null" : class_name, 
                 name == NULL ? "null" : name,  size);
        EXEC(node->call_bus(node, cmd, addr, &len));

        if (addr != NULL) {
            *addr = byteorder_be64_to_cpu(addr);
            dbg_str(DBG_SUC, "node alloc value_type:%d, name:%s, addr:%p", value_type, name, *addr);
        }
        THROW_IF(len != 8 && len != 4, -1);
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __mfree(Node *node, char *node_id, target_type_t type, char *name)
{
    char cmd[1024] = {0};
    int ret;
    
    TRY {
        snprintf(cmd, 1024, "node@mfree(%d, %s)", type, name == NULL ? "null" : name);
        EXEC(node->call_bus(node, cmd, NULL, 0));
        dbg_str(DBG_SUC, "node free name:%s", name);
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __mset(Node *node, char *node_id, target_type_t type, void *addr, int offset, int capacity, void *value, int value_len)
{
    char cmd[1024] = {0};
    int ret;
    
    TRY {
        THROW_IF(value_len > capacity, -1);
        snprintf(cmd, 1024, "node@mset(%d, 0x%p, %d, %d, 0x%p:%d)", type, addr, offset, capacity, value, value_len);
        EXEC(node->call_bus(node, cmd, NULL, 0));
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __mget(Node *node, char *node_id, target_type_t type, void *addr, int offset, int capacity, void *value, int *value_len)
{
    char cmd[1024] = {0};
    int ret;
    
    TRY {
        THROW_IF(*value_len > capacity, -1);
        snprintf(cmd, 1024, "node@mget(%d, 0x%p, %d, %d, %d)", type, addr, offset, capacity, *value_len);
        EXEC(node->call_bus(node, cmd, value, value_len));
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __mget_pointer(Node *node, char *node_id, target_type_t type, void *addr, void **dpointer)
{
    char cmd[1024] = {0};
    int ret, len;
    
    TRY {
        THROW_IF(addr == NULL || dpointer == NULL, -1);
        addr = byteorder_cpu_to_be64(&addr);
        len = sizeof(void *);
        snprintf(cmd, 1024, "node@mget_pointer(%d, 0x%p)", type, addr);
        EXEC(node->call_bus(node, cmd, dpointer, &len));
        *dpointer = byteorder_be64_to_cpu(dpointer);
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static class_info_entry_t node_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Node, construct, __construct),
    Init_Nfunc_Entry(2 , Node, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3 , Node, init, __init),
    Init_Nfunc_Entry(4 , Node, loop, __loop),
    Init_Nfunc_Entry(5 , Node, call_bus, __call_bus),
    Init_Nfunc_Entry(6 , Node, call_fsh, __call_fsh),
    Init_Nfunc_Entry(7 , Node, call_fsh_object_method, __call_fsh_object_method),
    Init_Nfunc_Entry(8 , Node, write_file, __write_file),
    Init_Nfunc_Entry(9 , Node, read_file, __read_file),
    Init_Nfunc_Entry(10, Node, copy, __copy),
    Init_Nfunc_Entry(11, Node, list, __list),
    Init_Nfunc_Entry(12, Node, malloc, __malloc),
    Init_Nfunc_Entry(13, Node, mfree, __mfree),
    Init_Nfunc_Entry(14, Node, mset, __mset),
    Init_Nfunc_Entry(15, Node, mget, __mget),
    Init_Nfunc_Entry(16, Node, mget_pointer, __mget_pointer),
    Init_End___Entry(17, Node),
};
REGISTER_CLASS(Node, node_class_info);

int node_find_method_argument_template(bus_object_t *obj, allocator_t *allocator, char *method_name, 
                                       bus_method_args_t **args, int *cnt)
{
    int i, ret, n_methods, n_policy;
    struct bus_method *method;
    struct blob_policy_s *policy = NULL;

    TRY {
        THROW_IF(obj == NULL || method_name == NULL || args == NULL, -1);

        dbg_str(DBG_VIP, "node_find_method_argument_template, method_name:%s", method_name);
        method = obj->methods;
        n_methods = obj->n_methods;

        for (i = 0; i < n_methods; i++) {
            method = obj->methods + i;
            if (strcmp(method->name, method_name) == 0) {
                policy = method->policy;
                n_policy = method->n_policy;
                break;
            }
        }

        if (cnt != NULL) *cnt = n_policy;
        THROW_IF(policy == NULL, 0);
        *args = allocator_mem_alloc(allocator, n_policy * sizeof(bus_method_args_t));
        THROW_IF(*args == NULL, -1);
        for (i = 0; i < n_policy; i++) {
            (*args + i)->type = (policy + i)->type;
            (*args + i)->name = allocator_mem_alloc(allocator, strlen((policy + i)->name) + 1);
            strcpy((*args + i)->name, (policy + i)->name);
            dbg_str(DBG_INFO, "method name:%s, type:%d", (policy + i)->name, (policy + i)->type);
        }
        
     } CATCH (ret) {}

    return ret;
}

int node_free_argument_template(allocator_t *allocator, bus_method_args_t *args, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        allocator_mem_free(allocator, (args + i)->name);
    }
    allocator_mem_free(allocator, args);

    return 1;
}
