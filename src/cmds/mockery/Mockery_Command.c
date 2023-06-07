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
    dbg_str(DBG_DETAIL, "mockery start");

    if (command->test_func_name != NULL && strcmp(command->test_func_name, "all") == 0) {
        execute_test_funcs();
    } else if (command->test_func_name != NULL) {
        execute_designated_test_func(command->test_func_name);
    } else if (command->cmd_name != NULL) {
        execute_designated_command(command->cmd_name, command->argc, command->argv);
    }

    dbg_str(DBG_DETAIL, "mockery end");
    debugger_set_all_businesses_level(debugger_gp, 1, 3);

    return 1;
}

static int __option_run_test_callback(Option *option, void *opaque)
{
    Mockery_Command *command = (Mockery_Command *)opaque;

    dbg_str(DBG_SUC,"option_run_test_callback:%s", STR2A(option->value));
    command->test_func_name = STR2A(option->value);

    return 1;
}

static int __option_run_cmd_callback(Option *option, void *opaque)
{
    Mockery_Command *command = (Mockery_Command *)opaque;

    dbg_str(DBG_SUC,"option_run_cmd_callback:%s", STR2A(option->value));
    command->cmd_name = STR2A(option->value);

    return 1;
}

static int __option_args_callback(Option *option, void *opaque)
{
    Mockery_Command *command = (Mockery_Command *)opaque;
    String *str;
    char *arg, **argv = command->argv;
    int cnt, i;

    dbg_str(DBG_SUC,"option_args_callback:%s", STR2A(option->value));
    str = option->value;
    cnt = str->split(str, "[,]", -1);
    for (i = 0; i < cnt; i++) {
        arg = str->get_splited_cstr(str, i);
        arg = str_trim(arg);
        argv[i] = arg;
        dbg_str(DBG_SUC, "cnt:%d, par i:%d value:%s", cnt, i, argv[i]);
    }
    command->argc = cnt;

    return 1;
}

static int __construct(Command *command, char *init_str)
{
    command->add_option(command, "--run-test", "", "", "run test funcs", __option_run_test_callback, command);
    command->add_option(command, "--run-cmd", "", "", "run cmd", __option_run_cmd_callback, command);
    command->add_option(command, "--args", "", "", "command args", __option_args_callback, command);
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
    char *str = "lbrnsepcfjzcpfgzqdiujo";
    char *p;
    int len, i;

    dbg_str(DBG_SUC, "test_mockery_command");
    for (i = 0; i < argc; i++) {
        dbg_str(DBG_SUC, "argc:%d, par i:%d value:%s", argc, i, argv[i]);
    }

    return 1;
}
REGISTER_TEST_CMD(test_mockery_command);

