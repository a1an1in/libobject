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
    return 0;
}

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
        dbg_str(DBG_VIP, "node init in");
        THROW_IF(node->host == NULL, -1);
        THROW_IF(node->service == NULL, -1);

        dbg_str(DBG_VIP,"node host:%s, service:%s", node->host, node->service);
        if (node->bus_deamon_flag == 1) {
            busd = busd_create(allocator, node->host,
                               node->service, SERVER_TYPE_INET_TCP);
            THROW_IF(busd == NULL, -1);
            node->busd = busd;
        }

        bus = bus_create(allocator, node->host,
                         node->service, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);
        node->bus = bus;
        
        bus_add_object(bus, &node_object);
        // bus_add_object(bus, &debug_object);

        // pause();
        sleep(10);
        
        // do { sleep(10); } while (node->node_flag != 1);

        dbg_str(DBG_VIP, "node node out");
    } CATCH (ret) {}

    return ret;
}

static class_info_entry_t node_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Node, construct, __construct),
    Init_Nfunc_Entry(2, Node, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Node, init, __init),
    Init_End___Entry(4, Node),
};
REGISTER_CLASS("Node", node_class_info);
