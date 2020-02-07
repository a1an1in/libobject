/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include <libobject/argument/Help_Command.h>

static int __run_action(Command *command)
{
    int argc, i;
    char **argv;

    dbg_str(DBG_DETAIL, "help command");

    argc = command->argc;
    argv = command->argv;

    for (i = 0; i < argc; i++) {
        dbg_str(DBG_DETAIL, "argv[%d]: %s", i, argv[i]);
    }

    dbg_str(DBG_DETAIL, "help command end");

    return 1;
}
static int __construct(Command *command, char *init_str)
{
    command->set(command, "/Command/name", "help");

    return 0;
}

static int __deconstruct(Command *command)
{
    return 0;
}

static class_info_entry_t help_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Help_Command, construct, __construct),
    Init_Nfunc_Entry(2, Help_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Help_Command, run_action, __run_action),
    Init_End___Entry(4, Help_Command),
};
REGISTER_APP_CMD("Help_Command", help_command_class_info);
