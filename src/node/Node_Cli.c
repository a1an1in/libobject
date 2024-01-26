/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include "Node_Cli.h"

static int __run_command(Node_Cli *command)
{
    int argc, i;
    char **argv;
    Command *c = (Command *)command;

    dbg_str(DBG_DETAIL, "node_cli command");

    argc = c->argc;
    argv = c->argv;

    for (i = 0; i < argc; i++) {
        dbg_str(DBG_DETAIL, "argv[%d]: %s", i, argv[i]);
    }

    dbg_str(DBG_DETAIL, "node_cli command end");

    return 1;
}
static int __construct(Node_Cli *command, char *init_str)
{
    command->set(command, "/Command/name", "node_cli");

    return 0;
}

static int __deconstruct(Node_Cli *command)
{
    return 0;
}

static class_info_entry_t node_cli_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Node_Cli, construct, __construct),
    Init_Nfunc_Entry(2, Node_Cli, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Node_Cli, set, NULL),
    Init_Nfunc_Entry(4, Node_Cli, get, NULL),
    Init_Vfunc_Entry(5, Node_Cli, run_command, __run_command),
    Init_End___Entry(6, Node_Cli),
};
REGISTER_APP_CMD("Node_Cli", node_cli_class_info);