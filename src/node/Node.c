/**
 * @file node.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-02-18
 */

#include <libobject/core/io/file_system_api.h>
#include <libobject/core/io/File.h>
#include <libobject/node/Node.h>

static int __construct(Node *node, char *init_str)
{
    allocator_t *allocator = node->parent.allocator;

    node->str = object_new(allocator, "String", NULL);
    
    return 0;
}

static int __deconstruct(Node *node)
{
    object_destroy(node->str);
    bus_destroy(node->bus);
    busd_destroy(node->busd);

    return 0;
}

static int __init(Node *node)
{
    allocator_t *allocator = node->parent.allocator;
    bus_t *bus = NULL;
    busd_t *busd = NULL;
    int i, ret;

    TRY {
        dbg_str(DBG_VIP, "node init in, obj addr:%p", &node_object);
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
        
        if (node->disable_node_service_flag != 1)
            bus_add_object(bus, &node_object);
        
        dbg_str(DBG_VIP, "node init out");
    } CATCH (ret) {}

    return ret;
}

static int __loop(Node *node)
{
    do { sleep(1); } while (node->node_flag != 1);
    dbg_str(DBG_VIP, "node loop out");
    return 0;
}

/* 
 * 这个地方约束了call对客户只支持node_object(默认调用的就是这个), 原因有下：
 * 1.如果有多个不同的object，客户端不知道客户使用的是哪个类型的object， 如果去
 *   bus查询，会比较麻烦， 还要把json转换成struct， 目前不支持这种转换， 如果
 *   要转Oject倒是容易，但是bus设计是在设计Object之前。 
 * 2.如果要提过多种不同的bus object class, 起始开发也有很大工作量， node 提供
 *   fshell的接口， 就不需要做额外的开发了， 直接就可以调用函数， 不用再对各种不同
 *   应用写不同bus接口了。
 */
static int __bus_call(Node *node, char *code, void *out, uint32_t *out_len)
{
    allocator_t *allocator = node->parent.allocator;
    char *method_name = NULL;
    char *node_id = NULL, *tmp;
    bus_method_args_t *args = NULL;
    String *str = node->str;
    int ret, argc, i, count;

    TRY {
        dbg_str(DBG_VIP, "bus_call in, code:%s", code);
        EXEC(str->reset(str));
        str->assign(str, code);
        count = str->split(str, "[,@ \t\n();]", -1);
        THROW_IF(count < 2, -1);

        node_id = str->get_splited_cstr(str, 0);
        method_name = str->get_splited_cstr(str, 1);
        EXEC(node_find_method_argument_template(&node_object, allocator, method_name, &args, &argc));
        THROW_IF(count - 2 != argc, -1); /* 除去node 和 方法名 */
        for (i = 0; i < argc; i++) {
            if (args[i].type == ARG_TYPE_UINT32) {
                args[i].value = atoi(str->get_splited_cstr(str, 2 + i));
            } else {
                args[i].value = str->get_splited_cstr(str, 2 + i);
            }
        }
        EXEC(bus_invoke_sync(node->bus, node_id, method_name, argc, args, out, out_len));
        node_free_argument_template(allocator, args, argc);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "bus_call argc:%d, count:%d", argc, count);
    }

    return ret;
}

/*
 * 这个不能复用bus_call, 因为execute不想把命令的参数也解析出来。如果加标记判断
 * 什么时候解析，会把bus_call搞复杂了。
 */
static int __execute(Node *node, char *code, void *out, uint32_t *out_len)
{
    bus_t *bus;
    String *str = node->str;
    char *node_id, *command;
    char buffer[1024] = {0};
    bus_method_args_t args[1] = {
        [0] = {ARG_TYPE_STRING, "command", NULL}, 
    };
    int ret, count;

    TRY {
        EXEC(str->reset(str));
        str->assign(str, code);
        count = str->split(str, "[@]", -1);
        THROW_IF(count != 2, -1);
        bus = node->bus;

        node_id = str->get_splited_cstr(str, 0);
        command = str->get_splited_cstr(str, 1);
        args[0].value = command;
        EXEC(bus_invoke_sync(bus, node_id, "exec_fshell", ARRAY_SIZE(args), args, NULL, NULL));
    } CATCH (ret) {}

    return ret;
}

static int __write(Node *node, char *from, char *node_id, char *to)
{
    bus_t *bus;
    File *file = NULL;
    allocator_t *allocator;
	char buffer[1024] = {0}, file_name;
    uint32_t buffer_size = sizeof(buffer);
    uint32_t size, i, read_len = 0, offset = 0, count = 0;
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
                EXEC(node->write(node, fs_file_info->file_name, node_id, buffer))
            }
        } else {
            dbg_str(DBG_VIP, "run at here, open file:%s", from);
            file = object_new(allocator, "File", NULL);
            EXEC(file->open(file, from, "r+"));
            size = fs_get_size(from);
            dbg_str(DBG_VIP, "run at here, size=%d, num:%d", size, size / buffer_size);
            for (i = 0; i <= (size / buffer_size); i++) {
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

static int __read(Node *node, char *node_id, char *from, char *to)
{
    bus_t *bus;
    File *file = NULL;
    allocator_t *allocator;
	char buffer[BLOB_BUFFER_MAX_SIZE] = {0}, file_name;
    uint32_t read_size = 1024;
    uint32_t size, i, read_len = 0, offset = 0, count = 0;
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
            dbg_str(DBG_VIP, "read file name:%s, size:%d", fs_file_info->file_name, size);
            
            EXEC(fs_get_relative_path(fs_file_info->file_name, from, &relative_path));
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, to);
            strcat(buffer, relative_path);
            dbg_str(DBG_VIP, "dest file name:%s", buffer);

            if (!fs_is_exist(buffer)) {
                EXEC(fs_mkfile(buffer, 0777));
            }
            EXEC(file->open(file, buffer, "a+"));
            for (i = 0; i <= (size / read_size); i++) {
                args[0].value = fs_file_info->file_name;
                args[1].value = offset;
                args[2].value = (size - i * read_size) > read_size ? read_size : (size - i * read_size);
                
                EXEC(bus_invoke_sync(bus, "node", "read_file", ARRAY_SIZE(args), args, buffer, &args[2].value));
                dbg_str(DBG_VIP, "read %s, offset:%d, read size:%d, read len:%d", 
                        from, offset, read_size, args[2].value);
                
                EXEC(file->seek(file, offset, SEEK_SET));
                EXEC(file->write(file, buffer, (int)args[2].value));
                offset += (uint32_t)args[2].value;
            }
            
            EXEC(file->close(file));
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
            EXEC(node->read(node, node_id, from , to));
        } else if (write_flag == 1) {
            EXEC(node->write(node, from, node_id, to));
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
        EXEC(node->bus_call(node, buffer, buffer, &len));
        THROW_IF(len == 0, 0);
        if (len > 0) {
            buffer[len] = 0;
        }

        vector->set(vector, "/Vector/value_type", &value_type);
        vector->set(vector, "/Vector/trustee_flag", &trustee_flag);
        vector->set(vector, "/Vector/value_to_json_callback", fs_file_info_struct_custom_to_json);
        vector->set(vector, "/Vector/value_free_callback", fs_file_info_struct_custom_free);
        vector->set(vector, "/Vector/value_new_callback", fs_file_info_struct_custom_new);
        vector->assign(vector, buffer);
    } CATCH (ret) {}

    return ret;
}

static class_info_entry_t node_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Node, construct, __construct),
    Init_Nfunc_Entry(2 , Node, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3 , Node, init, __init),
    Init_Nfunc_Entry(4 , Node, loop, __loop),
    Init_Nfunc_Entry(5 , Node, bus_call, __bus_call),
    Init_Nfunc_Entry(6 , Node, execute, __execute),
    Init_Nfunc_Entry(7 , Node, write, __write),
    Init_Nfunc_Entry(8 , Node, read, __read),
    Init_Nfunc_Entry(9 , Node, copy, __copy),
    Init_Nfunc_Entry(10, Node, list, __list),
    Init_End___Entry(11, Node),
};
REGISTER_CLASS("Node", node_class_info);

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
            dbg_str(DBG_VIP, "method name:%s, type:%d", (policy + i)->name, (policy + i)->type);
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
