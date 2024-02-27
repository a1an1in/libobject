/**
 * @file Command.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-05-19
 */
#include <libobject/core/io/Buffer.h>
#include <libobject/argument/Command.h>


static int __option_help_callback(Option *option, void *opaque)
{
    Command *command = (Command *)opaque;

    if (option->set_flag == 1) {
        command->help(command);
        exit(0);
    }

    return 1;
}

static int __option_log_level_callback(Option *option, void *opaque)
{
    Command *app = (Command *)opaque;
    uint32_t value, bussiness_num, level;
    int i, ret;

    TRY {
        RETURN_IF(option->set_flag != 1, 0);
        THROW_IF(option->value == NULL, -1);

        // value = atoi(STR2A(option->value));
        value = str_hex_to_int(STR2A(option->value));
        bussiness_num = (value >> 4) & 0xffffff;
        level = value & 0xf;

        dbg_str(DBG_VIP, "%s command set log level, value:%s, bussiness_num:%x, log level:%x", 
                STR2A(app->name), STR2A(option->value), bussiness_num, level);

        if (bussiness_num == 0) {
            debugger_set_all_businesses_level(debugger_gp, 1, atoi(STR2A(option->value)));
            THROW(1);
        }

        for(i = 0; i < MAX_DEBUG_BUSINESS_NUM; i++) {
            if ((bussiness_num >> i) & 1) debugger_set_business(debugger_gp, i, 1, level);
        }
    } CATCH (ret) { }
    
    return 1;
}

static int __construct(Command *command, char *init_str)
{
    return 0;
}

static int __deconstruct(Command *command)
{
    object_destroy(command->subcommands);
    object_destroy(command->options);
    object_destroy(command->args);
    object_destroy(command->name);
    object_destroy(command->description);

    return 0;
}

static int 
__add_subcommand(Command *command, void *command_name)
{
    Vector *subcommands = command->subcommands;
    int value_type = VALUE_TYPE_OBJ_POINTER;
    uint8_t trustee_flag = 1;
    char tmp[128] = {0};
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

    c = object_new(command->parent.allocator, command_name, NULL);
    if (c != NULL) {
        ret = subcommands->add(subcommands, c);
    } else {
        ret = -1;
    }
    snprintf(tmp,128, "%s help option", STR2A(c->name));
    c->add_option(c, "--help", "-h", NULL, tmp, __option_help_callback, c);
    c->add_option(c, "--log-level", "", "6", "setting log display level, the default value is 6.",
                      __option_log_level_callback, c);

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
        if (c->name->equal(c->name, command_name)) {
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
        if (o->name->equal(o->name, option_name) || 
            o->alias->equal(o->alias, option_name)) {
            return o;
        }
    }

    return NULL;
}

static int 
__add_argument(command, value, usage, action, opaque)
    Command *command;  char *value; char *usage; 
    int (*action)(Argument *,  void *); void *opaque;
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

    dbg_str(ARG_DETAIL, "add arg");

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
    int count, ret;
    Argument *arg;

    TRY {
        THROW_IF(args == NULL, -1);

        args->peek_at(args, index, (void **)&arg);
        if (arg == NULL) {
            command->add_argument(command, "", "", NULL, command);
            args->peek_at(args, index, (void **)&arg);
            THROW_IF(arg == NULL, -1);
            dbg_str(DBG_SUC, "arg addr %p", arg);
        }
    } CATCH (ret) { arg = NULL; }

    return arg;
}

static int __set_args(Command *command, int argc, char **argv)
{
    command->argc = argc;
    command->argv = argv;
}

static int
__parse_option_value_with_equal_sign(Command *command,
                                     char *option)
{
    String *str = NULL;
    Option *o;
    char *key = NULL, *value = NULL;
    int ret = 0, cnt, arg_cnt = 0, set_flag = 1;

    str = object_new(command->parent.allocator, "String", NULL);
    str->assign(str, option);
    cnt = str->split(str, "=", 2); 
    dbg_str(ARG_SUC, "key count :%d", cnt);
    if (cnt == 2) {
        key   = str->get_splited_cstr(str, 0);
        value = str->get_splited_cstr(str, 1);

        o = command->get_option(command, key);
        if (o != NULL) {
            o->set(o, "value", value);
            o->set(o, "set_flag", &set_flag);
            dbg_str(ARG_SUC, "set option key:%s, value:%s", key, value);
        }
    } 

    if (str != NULL) {
        object_destroy(str);
    }
}

static int 
__parse_option_value_with_space(Command *command, 
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
            v->append(v, ";", -1);
            v->append(v, value, -1);
            dbg_str(ARG_SUC, "set option key:%s, value:%s", key, value);
        } else {
            o->set(o, "value", value);
            o->set(o, "set_flag", &set_flag);
            dbg_str(ARG_SUC, "set option key:%s, value:%s", key, value);
        }
    }
}

static int __parse_option_with_no_value(Command *command, char *option)
{
    Option *o;
    uint8_t set_flag = 1;

    o = command->get_option(command, option);
    if (o != NULL) {
        // o->set(o, "value", "true");
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
    if ((o->value !=NULL)) {
        return 1;
    }

    return 0;
}

static int __parse_args(Command *command)
{
    void *p;
    int i;
    Command *c = NULL;
    int ret = 0, cnt, arg_cnt = 0;
    int set_flag = 1;
    Argument *argument;

    dbg_str(ARG_SUC, "parse command %s", command->argv[0]);

    if (command->argc <= 0) {
        return 0;
    }

    for (i = 0; i < command->argc; i++) {
        dbg_str(ARG_DETAIL, "argv[%d]:%s ", i, command->argv[i]);
    }

    p = strstr(command->argv[0], 
               command->name->get_cstr(command->name));
    if (p == NULL) {
        dbg_str(ARG_WARNNING, "args passed to wrong command");
        ret = -1;
    }

    for (i = 1; i < command->argc; i++) {
        if (__is_arg_option(command, command->argv[i])) {

            /*first process option with equal sign*/
            ret = __does_option_contains_equal_sign(command->argv[i]);
            if (ret != 0) {
                __parse_option_value_with_equal_sign(command, command->argv[i]);
                continue;
            }

            ret = __does_option_need_value(command, command->argv[i]);
            if (ret < 0) {
                dbg_str(ARG_WARNNING, "not recognized option");
                continue;
            } else if (ret == 0) { /*second, process option without value*/
                __parse_option_with_no_value(command, command->argv[i]);
                continue;
            }

            /*last process option need value, but pass with space*/
            if (!__is_arg_subcommand(command, command->argv[i + 1])) {
                __parse_option_value_with_space(command,
                        command->argv[i], command->argv[i + 1]);
                i++;
                continue;
            }
        } else {
            c = command->get_subcommand(command, command->argv[i]);
            if (c != NULL) {
                dbg_str(ARG_SUC, "%s found a command %s",
                        command->argv[0], command->argv[i]);
                c->set_args(c, command->argc - i, (char **)&command->argv[i]);
                c->parse_args(c);
                command->selected_subcommand = c;
                break;
            }

            if (command->args != NULL) {
                arg_cnt++;
                argument = command->get_argment(command, arg_cnt - 1);
                if (argument != NULL) {
                    dbg_str(ARG_SUC, "set arg %s",command->argv[i]);
                    argument->set(argument, "value", command->argv[i]);
                    argument->set(argument, "set_flag", &set_flag);
                } else {
                    dbg_str(ARG_WARNNING, "not recognize arg %s",command->argv[i]);
                }
            } else {
                dbg_str(ARG_WARNNING, "not recognize arg %s",command->argv[i]);
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

    if (options == NULL) return 0;

    option_count = options->count(options);
    for (i = 0; i < option_count; i++) {
        options->peek_at(options, i, (void **)&o);
        /* if configed default value and set_flag == 1, then need run option action */
        if ((o != NULL && o->action != NULL && o->value != NULL && strlen(STR2A(o->value)) > 0) ||
            o->set_flag == 1) {
            int (*option_action)(void *, void *) = o->action;
            option_action(o, o->opaque);
            dbg_str(DBG_DETAIL, "run option action:%s\n", o->name->get_cstr(o->name));
        }
    }
    return 0;
}

static int __run_argument_actions(Command *command)
{
    Argument *a;
    Vector *arguments = command->args;
    uint8_t i, argument_count;

    if (arguments == NULL) return 0;

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

static int __help_head_subcommand_for_each_callback(int index, void *element, void *arg)
{
    Command *command = (Command *)element;
    Buffer *buffer = (Buffer *)arg;
    void *addr;

    if (command == NULL) {
        return 0;
    }

    addr = buffer->rfind(buffer, "\n", 1);
    if (addr != NULL && (buffer->w_offset - (addr - buffer->addr) + strlen(STR2A(command->name)) > 80)) {
        buffer->printf(buffer, 512, "%s", "\n\t");
    }
    buffer->printf(buffer, 512, "%s | ", STR2A(command->name));

    return 0;
}

static int __help_head_option_for_each_callback(int index, void *element, void *arg)
{
    Option *option = (Option *)element;
    Buffer *buffer = (Buffer *)arg;
    void *addr;

    if (option == NULL) {
        return 0;
    }

    addr = buffer->rfind(buffer, "\n", 1);
    if (addr != NULL && (buffer->w_offset - (addr - buffer->addr) + strlen(STR2A(option->name)) > 80)) {
        buffer->printf(buffer, 512, "%s", "\n\t");
    }

    if (option->value != NULL) {
        buffer->printf(buffer, 512, "[%s|%s=<%s>] ", 
                       strlen(STR2A(option->alias)) > 0 ? STR2A(option->alias) : "n/a", 
                       strlen(STR2A(option->name)) > 0 ? STR2A(option->name) : "n/a", 
                       strlen(STR2A(option->value)) > 0 ? STR2A(option->value) : "n/a");
    } else {
        buffer->printf(buffer, 512, "[%s|%s] ", 
                       strlen(STR2A(option->alias)) > 0 ? STR2A(option->alias) : "n/a", 
                       strlen(STR2A(option->name)) > 0 ? STR2A(option->name) : "n/a");
    }

    return 0;
}

static int __help_head_argument_for_each_callback(int index, void *element, void *arg)
{
    Argument *argument = (Argument *)element;
    Buffer *buffer = (Buffer *)arg;
    void *addr;

    if (argument == NULL) {
        return 0;
    }

    addr = buffer->rfind(buffer, "\n", 1);
    if (addr != NULL && (buffer->w_offset - (addr - buffer->addr) + strlen("argx") > 80)) {
        buffer->printf(buffer, 512, "%s", "\n\t");
    }

    buffer->printf(buffer, 512, "<%s%d> ", "arg", index);

    return 0;
}

static int __help_details_option_for_each_callback(int index, void *element, void *arg)
{
    Option *option = (Option *)element;
    Buffer *buffer = (Buffer *)arg;
    void *addr;
    char tmp[50];

    if (option == NULL) {
        return 0;
    }

    snprintf(tmp, 50, "[%s|%s]", 
             strlen(STR2A(option->alias)) > 0 ? STR2A(option->alias) : "n/a", 
             strlen(STR2A(option->name)) > 0 ? STR2A(option->name) : "n/a");

    buffer->printf(buffer, 512, "%-30s\t%s\n", tmp, STR2A(option->usage));

    return 0;
}

static int __help_details_subcommand_for_each_callback(int index, void *element, void *arg)
{
    Command *command = (Command *)element;
    Buffer *buffer = (Buffer *)arg;
    char *desc;

    if (command == NULL) {
        return 0;
    }

    if (command->description) {
        desc = STR2A(command->description);
    } else {
        desc = "n/a";
    }
    buffer->printf(buffer, 1024, "%-30s\t%s\n", STR2A(command->name), desc);

    return 0;
}

static int __help_details_argument_for_each_callback(int index, void *element, void *arg)
{
    Argument *argument = (Argument *)element;
    Buffer *buffer = (Buffer *)arg;
    char *desc;
    char tmp[10] = {0};

    if (argument == NULL) {
        return 0;
    }

    if (argument->usage) {
        desc = STR2A(argument->usage);
    } else {
        desc = "n/a";
    }
    snprintf(tmp, 10, "%s%d", "arg", index);
    buffer->printf(buffer, 1024, "%-30s\t%s\n", tmp, desc);

    return 0;
}

static int __help(Command *command)
{
    allocator_t *allocator = command->parent.allocator;
    Vector *subcommands = command->subcommands;
    Vector *options = command->options;
    Vector *args = command->args;
    Buffer *buffer = NULL;
    int count, ret;

    TRY {
        buffer = OBJECT_NEW(allocator, Buffer, NULL);
        dbg_str(DBG_DETAIL, "run %s command help", STR2A(command->name));
        buffer->printf(buffer, 512, "usage:\n");
        buffer->printf(buffer, 512, "%s ", STR2A(command->name));

        options != NULL && options->for_each_arg(options, __help_head_option_for_each_callback, buffer);

        if (subcommands != NULL) {
            subcommands->for_each_arg(subcommands, __help_head_subcommand_for_each_callback, buffer);
            if (subcommands->count(subcommands) > 0) {
                buffer->w_offset -= 3;
            }
        } else if (args->count(args) != 0) {
            args->for_each_arg(args, __help_head_argument_for_each_callback, buffer);
        }

        if (options != NULL) {
            buffer->printf(buffer, 512, "\n\noptions details:\n");
            options->for_each_arg(options, __help_details_option_for_each_callback, buffer);
        }

        if (subcommands != NULL) {
            buffer->printf(buffer, 512, "\nsubcommands details:\n");
            subcommands->for_each_arg(subcommands, __help_details_subcommand_for_each_callback, buffer);
        } else if (args->count(args) != 0) {
            buffer->printf(buffer, 512, "\nargument details:\n");
            args->for_each_arg(args, __help_details_argument_for_each_callback, buffer);
        }
        
        printf("%s\n", (char *)buffer->addr);

    } CATCH (ret) {
    } FINALLY {
        object_destroy(buffer);
    }

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
    Init_Vfunc_Entry(12, Command, run_command, __action),
    Init_Vfunc_Entry(13, Command, run_option_actions, __run_option_actions),
    Init_Vfunc_Entry(14, Command, run_argument_actions, __run_argument_actions),
    Init_Vfunc_Entry(15, Command, set_args, __set_args),
    Init_Vfunc_Entry(16, Command, parse_args, __parse_args),
    Init_Vfunc_Entry(17, Command, help, __help),
    Init_Vec___Entry(18, Command, subcommands, NULL, "Test_Command"),
    Init_Vec___Entry(19, Command, options, NULL, "Option"),
    Init_Str___Entry(20, Command, name, NULL),
    Init_Str___Entry(21, Command, description, NULL),
    Init_Point_Entry(22, Command, opaque, NULL),
    Init_End___Entry(23, Command),
};
REGISTER_CLASS("Command", command_class_info);
