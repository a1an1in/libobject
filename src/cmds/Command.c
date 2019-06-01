/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/cmds/Command.h>

static int __construct(Command *command, char *init_str)
{
    return 0;
}

static int __deconstruct(Command *command)
{
    if (command->vector != NULL) {
        object_destroy(command->vector);
    }

    return 0;
}

int __add_subcommand(Command *command, void *subcommand)
{
    Vector *vector = command->vector;
    int ret = 0;

    if (vector == NULL) {
        vector = object_new(command->parent.allocator, 
                            "RBTree_Map", NULL);
        if (vector == NULL) {
            ret = -1;
            goto end;
        }
        command->vector = vector;
    }

    ret = vector->add(vector, subcommand);

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

static class_info_entry_t command_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Command, construct, __construct),
    Init_Nfunc_Entry(2 , Command, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3 , Command, get, NULL),
    Init_Nfunc_Entry(4 , Command, set, NULL),
    Init_Nfunc_Entry(5 , Command, to_json, NULL),
    Init_Nfunc_Entry(6 , Command, add_subcommand, __add_subcommand),
    Init_Nfunc_Entry(7 , Command, get_subcommand, __get_subcommand),
    Init_Vfunc_Entry(8 , Command, get_value, __get_value),
    Init_OP____Entry(9 , Command, vector, NULL),
    Init_End___Entry(10, Command),
};
REGISTER_CLASS("Command", command_class_info);
