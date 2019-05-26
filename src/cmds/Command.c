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
    Init_Nfunc_Entry(2, Command, get, NULL),
    Init_Nfunc_Entry(3, Command, set, NULL),
    Init_Nfunc_Entry(4, Command, to_json, NULL),
    Init_End___Entry(5, Command),
};
REGISTER_CLASS("Command", command_class_info);
