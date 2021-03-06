#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Vector.h>
#include <libobject/ctest/Test_Runner.h>
#include <libobject/cmds/Mockery_Command.h>
#include <libobject/argument/Application.h>


static int __run_action(Command *command)
{
    int argc;
    char **argv;
    dbg_str(DBG_DETAIL, "mockery start");

    argc = command->argc;
    argv = command->argv;
    if (argc == 2) {
        int targc = argc - 1;
        execute_test_designated_func(argv[1], (void *)&targc, argv + 1);
    } else {
        debugger_set_all_businesses_level(debugger_gp, 1, 6);
        execute_test_funcs();
    }

    dbg_str(DBG_DETAIL, "mockery end");

    return 1;
}


static int __construct(Command *command, char *init_str)
{
    command->set(command, "/Command/name", "mockery");

    return 0;
}

static int __deconstruct(Command *command)
{
    return 0;
}

static class_info_entry_t test_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Mockery_Command, construct, __construct),
    Init_Nfunc_Entry(2, Mockery_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Mockery_Command, run_action, __run_action),
    Init_End___Entry(4, Mockery_Command),
};
REGISTER_APP_CMD("Mockery_Command", test_command_class_info);
