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

    if (command->options != NULL) {
        object_destroy(command->options);
    }

    if (command->name != NULL) {
        object_destroy(command->name);
    }
    return 0;
}

static int __add_subcommand(Command *command, void *command_name)
{
    Vector *subcommands = command->subcommands;
    int value_type = VALUE_TYPE_OBJ_POINTER;
    uint8_t trustee_flag = 1;
    Command *c;
    int ret = 0;

    if (subcommands == NULL) {
        subcommands = object_new(command->parent.allocator, 
                                 "Vector", NULL);
        if (subcommands == NULL) {
            ret = -1;
            goto end;
        }
        subcommands->set(subcommands, "/Vector/value_type", &value_type);
        subcommands->set(subcommands, "/Vector/trustee_flag", &trustee_flag);
        command->subcommands = subcommands;
    }

    dbg_str(DBG_SUC, "add subcommand");

    c = object_new(command->parent.allocator, command_name, NULL);
    if (c != NULL) {
        ret = subcommands->add(subcommands, c);
    } else {
        ret = -1;
    }

end:
    return ret;
}

static Command * __get_subcommand(Command *command, char *command_name)
{
    return NULL;
}

static int 
__add_option(Command *command,
             char *name, char *alias, char *value,
             char *usage, int (*action)(void *, Option *))
{
    Vector *options = command->options;
    Option *o;
    int value_type = VALUE_TYPE_OBJ_POINTER;
    uint8_t trustee_flag = 1;
    int ret = 0;

    if (options == NULL) {
        options = object_new(command->parent.allocator, 
                                 "Vector", NULL);
        if (options == NULL) {
            ret = -1;
            goto end;
        }
        options->set(options, "/Vector/value_type", &value_type);
        options->set(options, "/Vector/trustee_flag", &trustee_flag);
        command->options = options;
    }

    dbg_str(DBG_SUC, "add option");

    o = object_new(command->parent.allocator, "Option", NULL);

    if (name != NULL)
        o->set(o, "name", name);
    if (alias != NULL)
        o->set(o, "alias", alias);
    if (usage != NULL)
        o->set(o, "usage", usage);
    if (value != NULL)
        o->set(o, "value", value);
    if (action != NULL)
        o->set(o, "action", action);

    ret = options->add(options, o);

end:
    return ret;
}

static Option *__get_option(Command *command, char *option_name)
{
    Vector *options = command->options;
}

static Option *__has_option(Command *command, char *option_name)
{
    Vector *options = command->options;
    int size = options->size(options);
    Option *o;
    int i;

    for (i = 0; i < size; i++) {
        options->peek_at(options, i, &o);
        if (    o->name->equal(o, option_name) || 
                o->alias->equal(o, option_name))
        {
            return o;
        }
    }

    return NULL;
}

static int __set_args(Command *command, int argc, char **argv)
{
    command->argc = argc;
    command->argv = argv;
}

static int __parse_args(Command *command)
{
    void *p;
    int i;

    dbg_str(DBG_SUC, "parse command");
    dbg_str(DBG_DETAIL, "argv0:%s argv1:%s", 
            command->argv[0], command->argv[1]);

    if (command->argc <= 0) {
        return 0;
    }

    p = strstr(command->argv[0], 
               command->name->get_cstr(command->name));
    if (p != NULL) {
        dbg_str(DBG_SUC, "args are mine");
        for (i = 0; i < command->argc; i++) {
        }

    } else {
        dbg_str(DBG_WARNNING, "args passed to wrong command");
        return -1;
    }

    return 1;
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
    Init_Vfunc_Entry(10, Command, has_option, __has_option),
    Init_Vfunc_Entry(11, Command, set_args, __set_args),
    Init_Vfunc_Entry(12, Command, parse_args, __parse_args),
    Init_Vec___Entry(13, Command, subcommands, NULL, "Test_Command"),
    Init_Vec___Entry(14, Command, options, NULL, "Option"),
    Init_Str___Entry(15, Command, name, NULL),
    Init_End___Entry(16, Command),
};
REGISTER_CLASS("Command", command_class_info);
