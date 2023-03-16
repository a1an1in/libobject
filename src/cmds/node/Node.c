/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include "Node.h"

static int __run_command(Node_Command *command)
{
    int argc, i;
    char **argv;
    Command *c = (Command *)command;

    dbg_str(ARG_DETAIL, "node command");

    argc = c->argc;
    argv = c->argv;

    for (i = 0; i < argc; i++) {
        dbg_str(ARG_DETAIL, "argv[%d]: %s", i, argv[i]);
    }

    dbg_str(ARG_DETAIL, "node command end");

    return 1;
}
static int __construct(Node_Command *command, char *init_str)
{
    command->set(command, "/Command/name", "node");

    return 0;
}

static int __deconstruct(Node_Command *command)
{
    return 0;
}

static class_info_entry_t node_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Node_Command, construct, __construct),
    Init_Nfunc_Entry(2, Node_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Node_Command, set, NULL),
    Init_Nfunc_Entry(4, Node_Command, get, NULL),
    Init_Vfunc_Entry(5, Node_Command, run_command, __run_command),
    Init_End___Entry(6, Node_Command),
};
REGISTER_APP_CMD("Node_Command", node_command_class_info);
