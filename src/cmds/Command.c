/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/cmds/Command.h>

static void *
__get_value(Command *command,char *command_name, char *flag_name)
{
}

static class_info_entry_t command_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Vfunc_Entry(1, Command, get_value, __get_value),
    Init_End___Entry(2, Command),
};
REGISTER_CLASS("Command", command_class_info);
