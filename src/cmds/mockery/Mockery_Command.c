#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/Vector.h>
#include <libobject/ctest/Test_Runner.h>
#include <libobject/cmds/Mockery_Command.h>
#include <libobject/argument/Application.h>

static int __run_command(Mockery_Command *command)
{
    dbg_str(DBG_VIP, "mockery %s start", command->func_name);

    if (command->func_name != NULL && strcmp(command->func_name, "all") == 0) {
        execute_test_funcs();
    } else if (command->argument_flag == 1) {
        execute_designated_func(command->func_name, command->argc, command->argv);
    }

    dbg_str(DBG_VIP, "mockery %s end", command->func_name);
    debugger_set_all_businesses_level(debugger_gp, 1, 6);

    return 1;
}

static int __argument_action_callback(Argument *argu, void *opaque)
{
    Mockery_Command *command = (Mockery_Command *)opaque;
    String *str;
    Argument *a;
    char *arg, **argv = command->argv;
    int cnt, i, ret;

    TRY {
        command->argument_flag = 1;
        command->func_name = STR2A(argu->value);
        cnt = command->parent.args->count(command->parent.args);
        dbg_str(DBG_SUC,"argument_action_callback:%s, args count:%d", STR2A(argu->value), cnt);

        for (i = 0; i < cnt; i++) {
            a = command->parent.get_argment(&command->parent, i);
            THROW_IF(a == NULL, -1);

            // arg = str_trim(STR2A(a->value));
            arg = STR2A(a->value);
            THROW_IF(arg == NULL, -1);
            argv[i] = arg;
            // dbg_str(DBG_SUC,"arg :%s", arg);  
        }
        command->argc = cnt;
    } CATCH (ret) {}

    return ret;
}

static int __construct(Command *command, char *init_str)
{
    command->add_argument(command, "", "function name to exec, which can be test func or command name", __argument_action_callback, command);
    command->set(command, "/Command/name", "mockery");
    command->set(command, "/Command/description", "mockery is used to do functional interface unit testing \n"
                 "                                and system testing. it can also run command like funtion.");

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
    Init_Vfunc_Entry(3, Mockery_Command, run_command, __run_command),
    Init_End___Entry(4, Mockery_Command),
};
REGISTER_APP_CMD("Mockery_Command", test_command_class_info);

int test_mockery_command(TEST_ENTRY *entry, int argc, char **argv)
{
    int len, i;

    dbg_str(DBG_SUC, "test_mockery_command");
    for (i = 0; i < argc; i++) {
        dbg_str(DBG_SUC, "argc:%d, par i:%d value:%s", argc, i, argv[i]);
    }

    return 1;
}
REGISTER_TEST_CMD(test_mockery_command);

