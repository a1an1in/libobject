/**
 * @file node.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-02-18
 */

#include "Node.h"

static int __deconstruct(Node *node)
{
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

        dbg_str(DBG_VIP,"node host:%s, service:%s", node->host, node->service);
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

static int __call(Node *node, char *code, void *out, uint32_t *out_len)
{
    allocator_t *allocator = node->parent.allocator;
    char *method_name = NULL;
    char *node_id = NULL;
    bus_method_args_t *args;
    int ret, count;

    TRY {
        dbg_str(DBG_VIP, "call in, code:%s", code);
        EXEC(node_find_method_argument_template(&node_object, allocator, method_name, &args, &count));
        EXEC(bus_invoke_sync(node->bus, node_id, method_name, count, args, out, &out_len));
    } CATCH (ret) {}

    return ret;
}

static class_info_entry_t node_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Node, construct, NULL),
    Init_Nfunc_Entry(2, Node, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Node, init, __init),
    Init_Nfunc_Entry(4, Node, loop, __loop),
    Init_Nfunc_Entry(5, Node, call, __call),
    Init_End___Entry(6, Node),
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
            (*args)->type = policy->type;
            strcpy((*args)->name, policy->name);
        }
        *cnt = n_policy;
     } CATCH (ret) {}

    return ret;
}
