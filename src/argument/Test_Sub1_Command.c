/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Test_Sub1_Command.h>

static int __construct(Command *command, char *init_str)
{
    return 0;
}

static int __deconstruct(Command *command)
{
    Test_Sub1_Command *test_sub1_command = (Test_Sub1_Command *)command;

    if (test_sub1_command->option != NULL)
        object_destroy(test_sub1_command->option);
    return 0;
}

static void *
__get_value(Command *command,char *command_name, char *flag_name)
{
}

static class_info_entry_t test_sub1_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Test_Sub1_Command, construct, __construct),
    Init_Nfunc_Entry(2, Test_Sub1_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Test_Sub1_Command, get_value, __get_value),
    Init_Str___Entry(4, Test_Sub1_Command, option, NULL),
    Init_U32___Entry(5, Test_Sub1_Command, help, 0),
    Init_End___Entry(6, Test_Sub1_Command),
};
REGISTER_CLASS("Test_Sub1_Command", test_sub1_command_class_info);
