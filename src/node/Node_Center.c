/**
 * @file Node_Center.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-01-26
 */

#include "Node_Center.h"

static int __construct(Node_Center *Node_Center, char *init_str)
{
    return 0;
}

static int __deconstruct(Node_Center *Node_Center)
{
    return 0;
}

static class_info_entry_t Node_Center_class_info[] = {
    Init_Obj___Entry(0, Node, parent),
    Init_Nfunc_Entry(1, Node_Center, construct, __construct),
    Init_Nfunc_Entry(2, Node_Center, deconstruct, __deconstruct),
    Init_End___Entry(3, Node_Center),
};
REGISTER_CLASS("Node_Center", Node_Center_class_info);
