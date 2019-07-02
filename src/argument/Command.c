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

    if (command->args != NULL) {
        object_destroy(command->args);
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

    dbg_str(DBG_DETAIL, "add subcommand");

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
    Vector *subcommands = command->subcommands;
    int count;
    Command *c;
    int i;

    if (subcommands == NULL) return NULL;

    count = subcommands->count(subcommands);

    for (i = 0; i < count; i++) {
        subcommands->peek_at(subcommands, i, (void **)&c);
        if (c->name->equal(c->name, command_name))  
        {
            return c;
        }
    }

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

    dbg_str(DBG_DETAIL, "add option");

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
    int count;
    Option *o;
    int i;

    if (options == NULL) return NULL;

    count = options->count(options);

    for (i = 0; i < count; i++) {
        options->peek_at(options, i, (void **)&o);
        if (    o->name->equal(o->name, option_name) || 
                o->alias->equal(o->alias, option_name))
        {
            return o;
        }
    }

    return NULL;
}

static int 
__add_argument(Command *command, char *value, char *usage)
{
    Vector *args = command->args;
    Argument *arg;
    int value_type = VALUE_TYPE_OBJ_POINTER;
    uint8_t trustee_flag = 1;
    int ret = 0;

    if (args == NULL) {
        args = object_new(command->parent.allocator, 
                             "Vector", NULL);
        if (args == NULL) {
            ret = -1;
            goto end;
        }
        args->set(args, "/Vector/value_type", &value_type);
        args->set(args, "/Vector/trustee_flag", &trustee_flag);
        command->args = args;
    }

    dbg_str(DBG_DETAIL, "add arg");

    arg = object_new(command->parent.allocator, "Argument", NULL);

    if (usage != NULL)
        arg->set(arg, "usage", usage);
    if (value != NULL)
        arg->set(arg, "value", value);

    ret = args->add(args, arg);

end:
    return ret;
}

static Argument *__get_argment(Command *command, int index)
{
    Vector *args = command->args;
    int count;
    Argument *arg;

    if (args == NULL) return NULL;

    args->peek_at(args, index, (void **)&arg);

    return arg;
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
    Command *c = NULL;
    Option *o;
    String *str = NULL;
    int ret = 0, cnt, arg_cnt = 0;
    char *key = NULL, *value = NULL;
    Argument *argument;

    dbg_str(DBG_SUC, "parse command %s", command->argv[0]);
    dbg_str(DBG_DETAIL, "argv0:%s argv1:%s, argv2:%s", 
            command->argv[0], command->argv[1], command->argv[2]);

    if (command->argc <= 0) {
        return 0;
    }

    p = strstr(command->argv[0], 
               command->name->get_cstr(command->name));
    if (p != NULL) {
        dbg_str(DBG_SUC, "args are mine");
        for (i = 1; i < command->argc; i++) {
            if (command->argv[i][0] == '-') {
                if (str == NULL) {
                    str = object_new(command->parent.allocator, "String", NULL);
                }
                str->assign(str, command->argv[i]);
                cnt = str->split_n(str, "=", 2); 
                dbg_str(DBG_SUC, "key count :%d", cnt);
                if (cnt == 2) {
                    key   = str->get_splited_cstr(str, 0);
                    value = str->get_splited_cstr(str, 1);

                    o = command->get_option(command, key);
                    if (o != NULL) {
                        o->set(o, "value", value);
                        dbg_str(DBG_SUC, "set option key:%s, value:%s", key, value);
                    }
                } 
                continue;
            } else {
                c = command->get_subcommand(command, command->argv[i]);
                if (c != NULL) {
                    dbg_str(DBG_SUC, "%s found a command %s",
                            command->argv[0], command->argv[i]);
                    c->set_args(c, command->argc - i, (char **)&command->argv[i]);
                    c->parse_args(c);
                    command->selected_subcommand = c;
                    break;
                } else {
                    if (command->args != NULL) {
                        arg_cnt++;
                        argument = command->get_argment(command, arg_cnt - 1);
                        if (argument != NULL) {
                            argument->set(argument, "value", command->argv[i]);
                        } else {
                            dbg_str(DBG_WARNNING, "not recognize arg %s",command->argv[i]);
                        }
                    } else {
                        dbg_str(DBG_WARNNING, "not recognize arg %s",command->argv[i]);
                    }
                }
            }
        }
    } else {
        dbg_str(DBG_WARNNING, "args passed to wrong command");
        ret = -1;
    }

    if (str != NULL) {
        object_destroy(str);
    }

    return ret;
}

static int __action(Command *command)
{
    return 0;
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
    Init_Vfunc_Entry(10, Command, add_argument, __add_argument),
    Init_Vfunc_Entry(11, Command, get_argment, __get_argment),
    Init_Vfunc_Entry(12, Command, action, __action),
    Init_Vfunc_Entry(13, Command, set_args, __set_args),
    Init_Vfunc_Entry(14, Command, parse_args, __parse_args),
    Init_Vec___Entry(15, Command, subcommands, NULL, "Test_Command"),
    Init_Vec___Entry(16, Command, options, NULL, "Option"),
    Init_Str___Entry(17, Command, name, NULL),
    Init_End___Entry(18, Command),
};
REGISTER_CLASS("Command", command_class_info);
