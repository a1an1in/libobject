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
    int argc, i;
    char **argv;
    Command *c = (Command *)command;
    allocator_t *allocator = c->parent.allocator;
    bus_t *bus;
    int ret;

    TRY {
        dbg_str(DBG_VIP,"node cli host:%s, service:%s", command->host, command->service);
        bus = bus_create(allocator, command->host, command->service, CLIENT_TYPE_INET_TCP);
        command->bus = bus;

        dbg_str(DBG_VIP, "Node cli command out");
    } CATCH (ret) {} FINALLY {}

    return ret;
}

static int __option_host_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;

    dbg_str(DBG_SUC,"option_host_action_callback:%s", STR2A(option->value));
    c->host = STR2A(option->value);

    return 1;
}

static int __option_service_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    dbg_str(DBG_SUC,"option_service_action_callback:%s", STR2A(option->value));
    c->service = STR2A(option->value);

    return 1;
}

static int __construct(Node_Cli_Command *command, char *init_str)
{
    Command *c = (Command *)command;

    c->set(c, "/Command/name", "node_cli");

    c->add_option(c, "--host", "-h", "", "set node center ip address", __option_host_callback, command);
    c->add_option(c, "--service", "-s", "", "set node center port", __option_service_callback, command);

    return 0;
}

static int __deconstruct(Node_Cli_Command *command)
{
    bus_destroy(command->bus);
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
