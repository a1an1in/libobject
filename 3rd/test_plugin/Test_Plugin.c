/**
 * @file Test_Plugin.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-07-20
 */
#include <libobject/argument/Application.h>
#include <libobject/mockery/mockery.h>
#include "Test_Plugin.h"

static int __construct(Command *command, char *init_str)
{
    int ret = 0, help = 0;

    help = 0;
    command->set(command, "/Test_Plugin/help", &help);
    command->set(command, "/Test_Plugin/option", "test command option");
    command->set(command, "/Command/name", "Test");

    command->add_subcommand(command, "Test_Sub1_Command");
    command->add_subcommand(command, "Test_Sub2_Command");

    return 0;
}

static int __deconstruct(Command *command)
{
    Test_Plugin *plugin = (Test_Plugin *)command;

    if (plugin->option != NULL)
        object_destroy(plugin->option);

    return 0;
}

static void *__get_value(Command *command,char *command_name, char *flag_name)
{
}

static class_info_entry_t Test_Plugin_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Test_Plugin, construct, __construct),
    Init_Nfunc_Entry(2, Test_Plugin, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Test_Plugin, get_value, __get_value),
    Init_Str___Entry(4, Test_Plugin, option, NULL),
    Init_U32___Entry(5, Test_Plugin, help, 0),
    Init_End___Entry(6, Test_Plugin),
};
REGISTER_APP_CMD(Test_Plugin, Test_Plugin_class_info);

