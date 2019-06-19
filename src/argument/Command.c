/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/argument/Command.h>

static int __construct(Command *command, char *init_str)
{
    return 0;
}

static int __deconstruct(Command *command)
{
    if (command->subcommands != NULL) {
        object_destroy(command->subcommands);
    }

    return 0;
}

int __add_subcommand(Command *command, void *subcommand)
{
    Vector *subcommands = command->subcommands;
    int value_type = VALUE_TYPE_OBJ_POINTER;
    int ret = 0;

    if (subcommands == NULL) {
        subcommands = object_new(command->parent.allocator, 
                                 "Vector", NULL);
        if (subcommands == NULL) {
            ret = -1;
            goto end;
        }
        subcommands->set(subcommands, "/Vector/value_type", &value_type);
        command->subcommands = subcommands;
    }

    dbg_str(DBG_SUC, "add subcommand");
    ret = subcommands->add(subcommands, subcommand);

end:
    return ret;
}

Command * __get_subcommand(Command *command, char *command_name)
{
    return NULL;
}

static void *
__get_value(Command *command,char *command_name, char *flag_name)
{
}

static int __add_option(Command *command, Option *option)
{
}

static Option *__get_option(Command *command, char *option_name)
{
}

static class_info_entry_t command_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Command, construct, __construct),
    Init_Nfunc_Entry(2 , Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Command, get, NULL),
    Init_Vfunc_Entry(4 , Command, set, NULL),
    Init_Vfunc_Entry(5 , Command, to_json, NULL),
    Init_Vfunc_Entry(6 , Command, add_subcommand, __add_subcommand),
    Init_Vfunc_Entry(7 , Command, get_subcommand, __get_subcommand),
    Init_Vfunc_Entry(8 , Command, add_option, __add_option),
    Init_Vfunc_Entry(9 , Command, get_option, __get_option),
    Init_Vec___Entry(10, Command, subcommands, NULL, "Test_Command"),
    Init_End___Entry(11, Command),
};
REGISTER_CLASS("Command", command_class_info);
