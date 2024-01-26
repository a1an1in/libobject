/**
 * @file node.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-01-26
 */

#include "Node.h"

static int __construct(Node *node, char *init_str)
{
    return 0;
}

static int __deconstruct(Node *node)
{
    return 0;
}

static class_info_entry_t node_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Node, construct, __construct),
    Init_Nfunc_Entry(2, Node, deconstruct, __deconstruct),
    Init_End___Entry(3, Node),
};
REGISTER_CLASS("Node", node_class_info);
