/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-02-18
 */

#include "Node_Cli_Command.h"

static int __run_command(Node_Cli_Command *command)
{
    Node *node = command->node;
    int ret;

    TRY {
        EXEC(node->init(node));
        EXEC(node->call(node, command->code, NULL, 0));
    } CATCH (ret) {}

    return ret; 
}

static int __option_host_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    Node *n = c->node;

    dbg_str(DBG_SUC,"option_host_action_callback:%s", STR2A(option->value));
    n->host = STR2A(option->value);

    return 1;
}

static int __option_service_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    Node *n = c->node;

    dbg_str(DBG_SUC,"option_service_action_callback:%s", STR2A(option->value));
    n->service = STR2A(option->value);

    return 1;
}

static int __option_code_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    dbg_str(DBG_SUC,"option_code_action_callback:%s", STR2A(option->value));
    c->code = STR2A(option->value);

    return 1;
}

static int __option_disable_node_service_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    Node *n = c->node;

    dbg_str(DBG_SUC,"disable_node_service: %s", STR2A(option->value));
    if ((strcmp(STR2A(option->value), "true") == 0) ||
        (strcmp(STR2A(option->value), "True") == 0) ||
        (strcmp(STR2A(option->value), "T") == 0) ||
        (strcmp(STR2A(option->value), "t") == 0)) {
        n->disable_node_service_flag = 1;
    } else {
        n->disable_node_service_flag = 0;
    }

    return 1;
}

static int __construct(Node_Cli_Command *command, char *init_str)
{
    Command *c = (Command *)command;
    allocator_t *allocator = c->parent.allocator;

    command->node = object_new(allocator, "Node", NULL);

    c->set(c, "/Command/name", "node_cli");
    c->add_option(c, "--host", "", "", "set node center ip address", __option_host_callback, command);
    c->add_option(c, "--service", "-s", "", "set node center port", __option_service_callback, command);
    c->add_option(c, "--call", "-c", "", "set the executing code of bus, eg, --call=nodeid@set_loglevel(1,2,3)", __option_code_callback, command);
    c->add_option(c, "--disable-node-service", "", "true", "disable node service for node cli", __option_disable_node_service_callback, command);
    c->set(c, "/Command/description", "node client command.");

    return 0;
}

static int __deconstruct(Node_Cli_Command *command)
{
    object_destroy(command->node);

    return 0;
}

static class_info_entry_t node_cli_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Node_Cli_Command, construct, __construct),
    Init_Nfunc_Entry(2, Node_Cli_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Node_Cli_Command, set, NULL),
    Init_Nfunc_Entry(4, Node_Cli_Command, get, NULL),
    Init_Vfunc_Entry(5, Node_Cli_Command, run_command, __run_command),
    Init_End___Entry(6, Node_Cli_Command),
};
REGISTER_APP_CMD("Node_Cli_Command", node_cli_command_class_info);
