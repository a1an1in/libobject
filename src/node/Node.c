/**
 * @file node.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-02-18
 */

#include "Node.h"

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
static int __call(Node *node, char *code, void *out, uint32_t *out_len)
{
    allocator_t *allocator = node->parent.allocator;
    char *method_name = NULL;
    char *node_id = NULL, *tmp;
    bus_method_args_t *args;
    String *str = node->str;
    int ret, argc, i, count;

    TRY {
        dbg_str(DBG_VIP, "call in, code:%s", code);
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
        EXEC(bus_invoke_sync(node->bus, node_id, method_name, argc, args, out, &out_len));
        node_free_argument_template(allocator, args, argc);
    } CATCH (ret) {}

    return ret;
}

static int __write(Node *node, char *from, char *node_id, char *to)
{

}

static int __read(Node *node, char *node_id, char *from, char *to)
{

}

static int __copy(Node *node, char *from, char *to)
{
    char *p1, *p2, *node_id;
    int read_flag = 0, write_flag = 0;
    int ret;

    TRY {
        p1 = strchr(from, ':');
        if (p1 != NULL) {
            read_flag = 1;
            node_id = from;
            *p1 = '\0';
            from = p1 + 1;
        }
        p2 = strchr(to, ':');
        if (p2 != NULL) {
            write_flag = 1;
            node_id = to;
            *p2 = '\0';
            to = p2 + 1;
        }
        THROW_IF(p1 == NULL && p2 == NULL, -1);
        THROW_IF(p1 != NULL && p2 != NULL, -1);
        dbg_str(DBG_VIP, "node copy, node:%s, from:%s, to:%s", node_id, from, to);

    } CATCH (ret) {}

    return ret;
}

static class_info_entry_t node_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Node, construct, __construct),
    Init_Nfunc_Entry(2, Node, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Node, init, __init),
    Init_Nfunc_Entry(4, Node, loop, __loop),
    Init_Nfunc_Entry(5, Node, call, __call),
    Init_Nfunc_Entry(6, Node, write, __write),
    Init_Nfunc_Entry(7, Node, read, __read),
    Init_Nfunc_Entry(8, Node, copy, __copy),
    Init_End___Entry(9, Node),
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

        THROW_IF(policy == NULL, 0);
        *args = allocator_mem_alloc(allocator, n_policy * sizeof(bus_method_args_t));
        THROW_IF(*args == NULL, -1);
        for (i = 0; i < n_policy; i++) {
            (*args + i)->type = (policy + i)->type;
            (*args + i)->name = allocator_mem_alloc(allocator, strlen((policy + i)->name) + 1);
            strcpy((*args + i)->name, (policy + i)->name);
            dbg_str(DBG_VIP, "method name:%s, type:%d", (policy + i)->name, (policy + i)->type);
        }
        *cnt = n_policy;
        
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
