/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-02-18
 */
#include <libobject/node/Node.h>
#include "Node_Command.h"

static int __run_command(Node_Command *command)
{
    Node *node = command->node;
    int ret;

    TRY {
        EXEC(node->init(node));
        EXEC(node->loop(node));
    } CATCH (ret) {}

    return ret; 
}

static int __option_host_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;
    Node *n = c->node;

    dbg_str(DBG_VIP,"option_host_action_callback:%s", STR2A(option->value));
    n->host = STR2A(option->value);

    return 1;
}

static int __option_service_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;
    Node *n = c->node;

    dbg_str(DBG_VIP,"option_service_action_callback:%s", STR2A(option->value));
    n->service = STR2A(option->value);

    return 1;
}

static int __option_deamon_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;
    Node *n = c->node;

    if ((strcmp(STR2A(option->value), "true") == 0) ||
        (strcmp(STR2A(option->value), "True") == 0) ||
        (strcmp(STR2A(option->value), "T") == 0) ||
        (strcmp(STR2A(option->value), "t") == 0)) {
        n->run_bus_deamon_flag = 1;
    } else {
        n->run_bus_deamon_flag = 0;
    }
    dbg_str(DBG_VIP, "set run_bus_deamon_flag:%d, option:%s", n->run_bus_deamon_flag, STR2A(option->value));
    
    return 1;
}

static int __construct(Node_Command *command, char *init_str)
{
    Command *c = (Command *)command;
    allocator_t *allocator = c->parent.allocator;

    command->node = object_new(allocator, "Node", NULL);
    
    c->set(c, "/Command/name", "node");
    c->add_option(c, "--host", "", "", "set node center ip address", __option_host_callback, command);
    c->add_option(c, "--service", "-s", "", "set node center port", __option_service_callback, command);
    c->add_option(c, "--deamon", "-d", "false", "run bus deamon", __option_deamon_callback, command);

    return 0;
}

static int __deconstruct(Node_Command *command)
{
    object_destroy(command->node);

    return 0;
}

static class_info_entry_t Node_Command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Node_Command, construct, __construct),
    Init_Nfunc_Entry(2, Node_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Node_Command, run_command, __run_command),
    Init_End___Entry(4, Node_Command),
};
REGISTER_APP_CMD(Node_Command, Node_Command_class_info);
