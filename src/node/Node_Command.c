/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-02-18
 */
#include <libobject/argument/Application.h>
#include "Node_Command.h"

static int __run_command(Node_Command *command)
{
    int argc, i;
    char **argv;
    Command *c = (Command *)command;

    dbg_str(DBG_VIP, "node command in");

    dbg_str(DBG_VIP,"node host:%s, service:%s", command->host, command->service);

    dbg_str(DBG_VIP, "node command out");

    return 1;
}

static int __option_host_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;

    dbg_str(DBG_SUC,"option_host_action_callback:%s", STR2A(option->value));
    c->host = STR2A(option->value);

    return 1;
}

static int __option_service_callback(Option *option, void *opaque)
{
    Node_Command *c = (Node_Command *)opaque;
    dbg_str(DBG_SUC,"option_service_action_callback:%s", STR2A(option->value));
    c->service = STR2A(option->value);

    return 1;
}

static int __construct(Node_Command *command, char *init_str)
{
    Command *c = (Command *)command;

    c->set(c, "/Command/name", "node");
    
    c->add_option(c, "--host", "-h", "", "set node center ip address", __option_host_callback, command);
    c->add_option(c, "--service", "-s", "", "set node center port", __option_service_callback, command);

    return 0;

    return 0;
}

static int __deconstruct(Node_Command *command)
{
    return 0;
}

static class_info_entry_t module_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Node_Command, construct, __construct),
    Init_Nfunc_Entry(2, Node_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Node_Command, set, NULL),
    Init_Nfunc_Entry(4, Node_Command, get, NULL),
    Init_Vfunc_Entry(5, Node_Command, run_command, __run_command),
    Init_End___Entry(6, Node_Command),
};
REGISTER_APP_CMD("Node_Command", module_command_class_info);
