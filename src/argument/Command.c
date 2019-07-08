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

static int 
__add_subcommand(Command *command, void *command_name)
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

static Command * 
__get_subcommand(Command *command, char *command_name)
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
             char *usage, int (*action)(Option *, void *),
             void *opaque)
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

    o->set(o, "command", command);
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
    if (opaque != NULL) 
        o->set(o, "opaque", opaque);

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
__add_argument(command, value, usage, action, opaque)
    Command *command; 
    char *value; 
    char *usage; 
    int (*action)(Argument *,  void *); 
    void *opaque;
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
    if (action != NULL)
        arg->set(arg, "action", action);
    if (opaque != NULL)
        arg->set(arg, "opaque", opaque);

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

static int
__parse_option_passing_value_with_equal_sign(Command *command,
                                             char *option)
{
    String *str = NULL;
    Option *o;
    char *key = NULL, *value = NULL;
    int ret = 0, cnt, arg_cnt = 0, set_flag = 1;

    str = object_new(command->parent.allocator, "String", NULL);
    str->assign(str, option);
    cnt = str->split_n(str, "=", 2); 
    dbg_str(DBG_SUC, "key count :%d", cnt);
    if (cnt == 2) {
        key   = str->get_splited_cstr(str, 0);
        value = str->get_splited_cstr(str, 1);

        o = command->get_option(command, key);
        if (o != NULL) {
            o->set(o, "value", value);
            o->set(o, "set_flag", &set_flag);
            dbg_str(DBG_SUC, "set option key:%s, value:%s", key, value);
        }
    } 

    if (str != NULL) {
        object_destroy(str);
    }
}

static int 
__parse_option_passing_value_with_space(Command *command, 
                                        char *key, char *value)
{
    Option *o;
    uint8_t set_flag = 1, multi_value_flag = 1, *flag;
    String *v;

    o = command->get_option(command, key);
    if (o != NULL) {
        flag = o->get(o, "set_flag");
        if (*flag == 1) {
            v = o->value;
            o->set(o, "multi_value_flag", &multi_value_flag);
            v->append(v, ";");
            v->append(v, value);
            dbg_str(DBG_SUC, "set option key:%s, value:%s", key, value);
        } else {
            o->set(o, "value", value);
            o->set(o, "set_flag", &set_flag);
            dbg_str(DBG_SUC, "set option key:%s, value:%s", key, value);
        }
    }
}

static int __parse_option_with_no_value(Command *command, char *option)
{
    Option *o;
    uint8_t set_flag = 1;

    o = command->get_option(command, option);
    if (o != NULL) {
        o->set(o, "value", "true");
        o->set(o, "set_flag", &set_flag);
    }
}

static int __does_option_contains_equal_sign(char *option)
{
    char *p;

    p = strstr(option, "=");
    if(NULL !=  p) {
        return p - option;
    }

    return 0;
}

static int __is_arg_subcommand(Command *command, char *arg)
{
    Command *c = NULL;

    c = command->get_subcommand(command, arg);
    if (c != NULL) {
        return 1;
    }

    return 0;
}

static int __is_arg_option(Command *command, char *arg)
{
    if (arg[0] == '-') {
        return 1;
    }

    return 0;
}

static int __does_option_need_value(Command *command, char *option)
{
    Option *o;

    o = command->get_option(command, option);

    if (o == NULL) return -1;
    if (o != NULL && (o->value->equal(o->value, "true") ||
                      o->value->equal(o->value, "True") ||
                      o->value->equal(o->value, "false") ||
                      o->value->equal(o->value, "False"))) 
    {
        return 0;
    }

    return 1;
}

static int __parse_args(Command *command)
{
    void *p;
    int i;
    Command *c = NULL;
    int ret = 0, cnt, arg_cnt = 0;
    int set_flag = 1;
    Argument *argument;

    dbg_str(DBG_SUC, "parse command %s", command->argv[0]);

    if (command->argc <= 0) {
        return 0;
    }

    for (i = 0; i < command->argc; i++) {
        dbg_str(DBG_DETAIL, "argv[%d]:%s ", i, command->argv[i]);
    }

    p = strstr(command->argv[0], 
               command->name->get_cstr(command->name));
    if (p == NULL) {
        dbg_str(DBG_WARNNING, "args passed to wrong command");
        ret = -1;
    }

    for (i = 1; i < command->argc; i++) {
        if (__is_arg_option(command, command->argv[i])) {

            /*first process option with equal sign*/
            ret = __does_option_contains_equal_sign(command->argv[i]);
            if (ret != 0) {
                __parse_option_passing_value_with_equal_sign(command, command->argv[i]);
                continue;
            }

            ret = __does_option_need_value(command, command->argv[i]);
            if (ret < 0) {
                dbg_str(DBG_WARNNING, "not recognized option");
                continue;
            }

            /*second, process option without value*/
            if (ret == 0) {
                __parse_option_with_no_value(command, command->argv[i]);
                continue;
            }

            /*last process option need value, but pass with space*/
            if (!__is_arg_subcommand(command, command->argv[i + 1])) {
                __parse_option_passing_value_with_space(command,
                        command->argv[i], command->argv[i + 1]);
                i++;
                continue;
            }
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
                        dbg_str(DBG_SUC, "set arg %s",command->argv[i]);
                        argument->set(argument, "value", command->argv[i]);
                        argument->set(argument, "set_flag", &set_flag);
                    } else {
                        dbg_str(DBG_WARNNING, "not recognize arg %s",command->argv[i]);
                    }
                } else {
                    dbg_str(DBG_WARNNING, "not recognize arg %s",command->argv[i]);
                }
            }
        }
    }

    return ret;
}

static int __run_option_actions(Command *command)
{
    Option *o;
    Vector *options = command->options;
    uint8_t i, option_count;

    option_count = command->options->count(command->options);

    for (i = 0; i < option_count; i++) {
        options->peek_at(options, i, (void **)&o);
        if (o != NULL && o->set_flag == 1 && o->action != NULL) {
            int (*option_action)(void *, void *) = o->action;
            option_action(o, o->opaque);
        }
    }
    return 0;
}

static int __run_argument_actions(Command *command)
{
    Argument *a;
    Vector *arguments = command->args;
    uint8_t i, argument_count;

    argument_count = arguments->count(arguments);

    for (i = 0; i < argument_count; i++) {
        arguments->peek_at(arguments, i, (void **)&a);
        if (a != NULL && a->set_flag == 1 && a->action != NULL) {
            int (*argument_action)(void *, void *) = a->action;
            argument_action(a, a->opaque);
        }
    }
    return 0;
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
    Init_Vfunc_Entry(12, Command, run_action, __action),
    Init_Vfunc_Entry(13, Command, run_option_actions, __run_option_actions),
    Init_Vfunc_Entry(14, Command, run_argument_actions, __run_argument_actions),
    Init_Vfunc_Entry(15, Command, set_args, __set_args),
    Init_Vfunc_Entry(16, Command, parse_args, __parse_args),
    Init_Vec___Entry(17, Command, subcommands, NULL, "Test_Command"),
    Init_Vec___Entry(18, Command, options, NULL, "Option"),
    Init_Str___Entry(19, Command, name, NULL),
    Init_Point_Entry(20, Command, opaque, NULL),
    Init_End___Entry(21, Command),
};
REGISTER_CLASS("Command", command_class_info);
