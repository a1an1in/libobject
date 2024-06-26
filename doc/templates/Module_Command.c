/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include "Module_Command.h"

static int __run_command(Module_Command *command)
{
    int argc, i;
    char **argv;
    Command *c = (Command *)command;

    dbg_str(DBG_VIP, "module command");

    argc = c->argc;
    argv = c->argv;

    for (i = 0; i < argc; i++) {
        dbg_str(DBG_VIP, "argv[%d]: %s", i, argv[i]);
    }

    dbg_str(DBG_VIP, "module command end");

    return 1;
}
static int __construct(Module_Command *command, char *init_str)
{
    command->set(command, "/Command/name", "module");

    return 0;
}

static int __deconstruct(Module_Command *command)
{
    return 0;
}

static class_info_entry_t module_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Module_Command, construct, __construct),
    Init_Nfunc_Entry(2, Module_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Module_Command, set, NULL),
    Init_Nfunc_Entry(4, Module_Command, get, NULL),
    Init_Vfunc_Entry(5, Module_Command, run_command, __run_command),
    Init_End___Entry(6, Module_Command),
};
REGISTER_APP_CMD(Module_Command, module_command_class_info);
