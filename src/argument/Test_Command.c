/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Application.h>
#include <libobject/argument/Test_Command.h>
#include <libobject/argument/Test_Sub1_Command.h>
#include <libobject/argument/Test_Sub2_Command.h>
#include <libobject/mockery/mockery.h>

static int __construct(Command *command, char *init_str)
{
    int ret = 0, help = 0;

    help = 0;
    command->set(command, "/Test_Command/help", &help);
    command->set(command, "/Test_Command/option", "test command option");
    command->set(command, "/Command/name", "test");

    command->add_subcommand(command, "Test_Sub1_Command");
    command->add_subcommand(command, "Test_Sub2_Command");

    return 0;
}

static int __deconstruct(Command *command)
{
    Test_Command *test_command = (Test_Command *)command;

    if (test_command->option != NULL)
        object_destroy(test_command->option);

    return 0;
}

static void *
__get_value(Command *command,char *command_name, char *flag_name)
{
}

static class_info_entry_t test_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Test_Command, construct, __construct),
    Init_Nfunc_Entry(2, Test_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Test_Command, get_value, __get_value),
    Init_Str___Entry(4, Test_Command, option, NULL),
    Init_U32___Entry(5, Test_Command, help, 0),
    Init_End___Entry(6, Test_Command),
};
REGISTER_APP_CMD(Test_Command, test_command_class_info);

