/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include <libobject/scripts/FShell.h>
#include "FShell_Command.h"

static int __run_action(Command *command)
{
    int argc, i, count;
    char **argv;
    Command *c = (Command *)command;
    Argument *arg;
    FShell *shell = ((FShell_Command *)command)->shell;

    dbg_str(DBG_DETAIL, "fshell run");

    count = command->args->count(command->args);
    dbg_str(DBG_SUC,"arg count =%d", count);
    if (count > 1) {
        arg = command->get_argment(command, 0);
        dbg_str(DBG_SUC,"arg0:%s", arg->value->get_cstr(arg->value));
    }
    shell->open_ui(shell);

    dbg_str(DBG_DETAIL, "fshell out");

    return 1;
}

static int __construct(Command *command, char *init_str)
{
    FShell_Command *fcommand = (FShell_Command *)command;

    allocator_t *allocator = command->parent.allocator;

    command->set(command, "/Command/name", "fshell");
    command->add_argument(command, "", "arg0", NULL, NULL);
    command->add_argument(command, "", "arg1", NULL, NULL);
    fcommand->shell = object_new(allocator, "UnixFShell", NULL);

    return 0;
}

static int __deconstruct(FShell_Command *command)
{
    object_destroy(command->shell);

    return 0;
}

static class_info_entry_t FShell_Command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, FShell_Command, construct, __construct),
    Init_Nfunc_Entry(2, FShell_Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, FShell_Command, set, NULL),
    Init_Nfunc_Entry(4, FShell_Command, get, NULL),
    Init_Vfunc_Entry(5, FShell_Command, run_action, __run_action),
    Init_End___Entry(6, FShell_Command),
};
REGISTER_APP_CMD("FShell_Command", FShell_Command_class_info);