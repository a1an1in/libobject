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
        if (command->command_type == COMMAND_TYPE_BUS_CALL) {
            EXEC(node->call(node, command->arg1, NULL, 0));
        } else if (command->command_type == COMMAND_TYPE_FSHELL_CALL) {
        }
        
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

static int __option_bus_call_command_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    dbg_str(DBG_SUC,"option_bus_cmd_action_callback:%s", STR2A(option->value));
    c->arg1 = STR2A(option->value);
    c->command_type = COMMAND_TYPE_BUS_CALL;

    return 1;
}

static int __option_fshell_call_command_callback(Option *option, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;
    dbg_str(DBG_SUC,"option_fshell_cmd_action_callback:%s", STR2A(option->value));
    c->arg1 = STR2A(option->value);
    c->command_type = COMMAND_TYPE_FSHELL_CALL;

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

static int __argument_arg0_action_callback(Argument *arg, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;

    dbg_str(DBG_SUC,"argument arg0:%s", STR2A(arg->value));

    if (strcmp(STR2A(arg->value), "bus_call") == 0) {
        c->command_type = COMMAND_TYPE_BUS_CALL;
    } else if (strcmp(STR2A(arg->value), "fshell_call") == 0) {
        c->command_type = COMMAND_TYPE_FSHELL_CALL;
    }

    return 0;
}

static int __argument_arg1_action_callback(Argument *arg, void *opaque)
{
    Node_Cli_Command *c = (Node_Cli_Command *)opaque;

    dbg_str(DBG_SUC,"argument arg1:%s", STR2A(arg->value));
    c->arg1 = STR2A(arg->value);

    return 0;
}

static int __construct(Node_Cli_Command *command, char *init_str)
{
    Command *c = (Command *)command;
    allocator_t *allocator = c->parent.allocator;

    command->node = object_new(allocator, "Node", NULL);

    c->set(c, "/Command/name", "node_cli");
    c->add_option(c, "--host", "", "", "set node center ip address.", __option_host_callback, command);
    c->add_option(c, "--service", "-s", "", "set node center port.", __option_service_callback, command);
    c->add_option(c, "--bus_call", "-b", "",
                                                    "set the executing code of bus, \n"
                  "                                ""eg: --bus_call=nodeid@set_loglevel(1,2,3).",
                  __option_bus_call_command_callback, command);
    c->add_option(c, "--fshell_call", "-f", "", "set the executing code of fshell, eg, --fshell_call=fsh_add(1,2).", 
                  __option_fshell_call_command_callback, command);
    c->add_option(c, "--disable-node-service", "", "true", "disable node service for node cli.", 
                  __option_disable_node_service_callback, command);
    c->add_argument(c, "",
                                                      "command type, it's optional if you want to call bus or fshell, we can excute cmd by option.\n"
                    "                                ""now we support bus_call, fshell_call and copy commands.",
                    __argument_arg0_action_callback, command);
    c->add_argument(c, "",
                                                      "if arg0 is bus_call or fshell_call, it should set call method here, \n"
                    "                                ""like nodeid@set_loglevel(1,2,3).\n"
                    "                                ""if arg0 is copy command, it should set the source file to copy.\n"
                    "                                ""if arg0 is upgrade command, it should set install package URL.",
                    __argument_arg1_action_callback, command);
    c->add_argument(c, "", "only copy command need arg2, which represent where the file to copy to.\n", NULL, NULL);
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
