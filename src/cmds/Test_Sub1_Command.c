/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/cmds/Test_Sub1_Command.h>

static void *
__get_value(Command *command,char *command_name, char *flag_name)
{
}

static class_info_entry_t test_sub1_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Vfunc_Entry(1, Test_Sub1_Command, get_value, __get_value),
    Init_Str___Entry(2, Test_Sub1_Command, option, NULL),
    Init_U32___Entry(3, Test_Sub1_Command, help, 0),
    Init_End___Entry(4, Test_Sub1_Command),
};
REGISTER_CLASS("Test_Sub1_Command", test_sub1_command_class_info);
