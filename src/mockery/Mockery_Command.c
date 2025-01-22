#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Vector.h>
#include <libobject/core/Struct_Adapter.h>
#include <libobject/argument/Application.h>
#include <libobject/mockery/Mockery_Command.h>
#include <libobject/mockery/mockery.h>

static int mockery_result_struct_custom_to_json(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    init_func_entry_t *o = (init_func_entry_t *)element;

    item = cjson_create_object();
    cjson_add_string_to_object(item, "func_name", o->func_name);
    cjson_add_string_to_object(item, "file_name", o->file);
    cjson_add_number_to_object(item, "line", o->line);
    if (item != NULL) {
        cjson_add_item_to_array(root, item);
    }

    return 1;
}

static int mockery_result_struct_custom_free(allocator_t *allocator, init_func_entry_t *value)
{
    free(value);

    return 1;
}

/**
 * 这个函数会运行所有的mockery test functions.
 */
int execute_test_funcs(Mockery_Command *command) 
{
    int i, size = 0, ret;
    init_func_entry_t *element;
    Vector *failed;
    reg_heap_t * reg_heap = get_global_testfunc_reg_heap();

    size = reg_heap_size(reg_heap);
    failed = command->failed_cases;
    for (i = 0; i < size; i++) {
        reg_heap_remove(reg_heap, (void **)&element);

        if (element->type == FUNC_ENTRY_TYPE_CMD) {
            free(element);
            continue;
        }
        if (element->args_count == 1) {
            ret = element->func1((void *)element);
            if (ret <= 0) {
                dbg_str(DBG_ERROR, "test failed, func_name = %s,  file = %s, line = %d", 
                        element->func_name, element->file, element->line);
                failed->add(failed, element);    
            } else {
                dbg_str(DBG_VIP, "test suc, func_name = %s,  file = %s, line = %d", 
                        element->func_name, element->file, element->line);
                free(element);
            }
        } else { }   
    }

    reg_heap_destroy(reg_heap);

    return 0;
}

/**
 * 这个函数只会运行匹配的mockery test functions.
 */
int execute_designated_func(Mockery_Command *command, char *func_name, int arg1, char **arg2) 
{
    int i, size = 0, ret;
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_testfunc_reg_heap();
    int flag = 0;

    size = reg_heap_size(reg_heap);
    for (i = 0; i < size; i++) {
        reg_heap_remove(reg_heap, (void **)&element);

        if ((element->type == FUNC_ENTRY_TYPE_CMD) || 
            (strncmp(element->func_name, func_name, strlen(func_name)) != 0)) {
            free(element);
            continue;
        }

        if (element->args_count == 1) {
            ret = element->func1((void *)element);
            flag = 1;
        } else if (element->args_count == 3) {
            ret = element->func3((void *)element, arg1, arg2);
            flag = 1;
        } else {
            free(element);
            continue;
        }

        if (ret <= 0) {
            dbg_str(DBG_ERROR, "command failed, func_name = %s,  file = %s, line = %d", 
                    element->func_name, element->file, element->line);
        } else {
            dbg_str(DBG_WIP, "command suc, func_name = %s,  file = %s, line = %d", 
                    element->func_name, element->file, element->line);
        }
        free(element);
    }

    if (flag == 0) {
        dbg_str(DBG_ERROR, "not found func_name = %s register map", func_name);
    }

    reg_heap_destroy(reg_heap);

    return 0;
}

/**
 * 这个函数只会运行匹配的mockery test command.
 */
int execute_designated_cmd(Mockery_Command *command, char *func_name, int arg1, char **arg2) 
{
    int i, size = 0, ret;
    init_func_entry_t *element;
    reg_heap_t * reg_heap = get_global_testfunc_reg_heap();
    int flag = 0;

    size = reg_heap_size(reg_heap);
    for (i = 0; i < size; i++) {
        reg_heap_remove(reg_heap, (void **)&element);

        if ((element->type != FUNC_ENTRY_TYPE_CMD) || 
            (strncmp(element->func_name, func_name, strlen(func_name)) != 0)) {
            free(element);
            continue;
        }

        if (element->args_count == 1) {
            ret = element->func1((void *)element);
            flag = 1;
        } else if (element->args_count == 3) {
            ret = element->func3((void *)element, arg1, arg2);
            flag = 1;
        } else {
            free(element);
            continue;
        }

        if (ret <= 0) {
            dbg_str(DBG_ERROR, "command failed, func_name = %s,  file = %s, line = %d", 
                    element->func_name, element->file, element->line);
        } else {
            dbg_str(DBG_VIP, "command suc, func_name = %s,  file = %s, line = %d", 
                    element->func_name, element->file, element->line);
        }
        free(element);
    }

    if (flag == 0) {
        dbg_str(DBG_ERROR, "not found func_name = %s register map", func_name);
    }

    reg_heap_destroy(reg_heap);

    return 0;
}

static int __run_command(Mockery_Command *command)
{
    Vector *failed = command->failed_cases;

    dbg_str(DBG_VIP, "mockery %s start", command->func_name);

    /* -f和arg为all， 则运行所有test */
    if (command->func_name != NULL && strcmp(command->func_name, "all") == 0 && command->function_flag == 1) {
        execute_test_funcs(command);
        dbg_str(DBG_VIP, "mockery test failed results:%s", failed->to_json(failed));
    /* -f和arg为test函数名， 则运行匹配的mockery test */
    } else if (command->argument_flag == 1 && command->function_flag == 1) {
        execute_designated_func(command, command->func_name, command->argc, command->argv);
    /* 只指定arg， 则运行匹配的mockery cmd */
    } else if (command->argument_flag == 1 && command->function_flag != 1) {
        execute_designated_cmd(command, command->func_name, command->argc, command->argv);
    }

    dbg_str(DBG_VIP, "mockery %s end", command->func_name);
    debugger_set_all_businesses_level(debugger_gp, 1, 4);

    return 1;
}

static int __option_mockery_function_callback(Option *option, void *opaque)
{
    Mockery_Command *c = (Mockery_Command *)opaque;
    c->function_flag = 1;

    return 1;
}

static int __argument_action_callback(Argument *argu, void *opaque)
{
    Mockery_Command *command = (Mockery_Command *)opaque;
    String *str;
    Argument *a;
    char *arg, **argv = command->argv;
    int cnt, i, ret;

    TRY {
        command->argument_flag = 1;
        command->func_name = STR2A(argu->value);
        cnt = command->parent.args->count(command->parent.args);
        // dbg_str(DBG_VIP,"argument_action_callback:%s, args count:%d", STR2A(argu->value), cnt);

        for (i = 0; i < cnt; i++) {
            a = command->parent.get_argment(&command->parent, i);
            THROW_IF(a == NULL, -1);

            // arg = str_trim(STR2A(a->value));
            arg = STR2A(a->value);
            THROW_IF(arg == NULL, -1);
            argv[i] = arg;
            // dbg_str(DBG_VIP,"arg :%s", arg);  
        }
        command->argc = cnt;
    } CATCH (ret) {}

    return ret;
}

static int __construct(Mockery_Command *mockery, char *init_str)
{
    Command *command = (Command *)mockery;
    allocator_t *allocator = command->parent.allocator;
    Vector *failed;
    uint8_t trustee_flag = 1;
    uint8_t value_type = VALUE_TYPE_STRUCT_POINTER;

    command->add_option(command, "--function", "-f", NULL, "run mockery test function", __option_mockery_function_callback, command);
    command->add_argument(command, "", "function name to exec, which can be test func or command name", __argument_action_callback, command);
    command->set(command, "/Command/name", "mockery");
    command->set(command, "/Command/description", "mockery is used to do functional interface unit testing \n"
                 "                                and system testing. it can also run command like funtion.");

    failed  = object_new(allocator, "Vector", NULL);
    failed->set(failed, "/Vector/trustee_flag", &trustee_flag);
    failed->set(failed, "/Vector/value_type", &value_type);
    failed->set(failed, "/Vector/class_name", "Mockery_Result_Struct_Adapter");

    mockery->failed_cases = failed;

    return 0;
}

static int __deconstruct(Mockery_Command *mockery)
{
    object_destroy(mockery->failed_cases);

    return 0;
}

static class_info_entry_t test_command_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Mockery_Command, construct, __construct),
    Init_Nfunc_Entry(2, Mockery_Command, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Mockery_Command, run_command, __run_command),
    Init_End___Entry(4, Mockery_Command),
};
REGISTER_APP_CMD(Mockery_Command, test_command_class_info);

int test_mockery_command(TEST_ENTRY *entry, int argc, char **argv)
{
    int len, i;

    dbg_str(DBG_VIP, "test_mockery_command");
    for (i = 0; i < argc; i++) {
        dbg_str(DBG_VIP, "argc:%d, par i:%d value:%s", argc, i, argv[i]);
    }

    return 1;
}
REGISTER_TEST_CMD(test_mockery_command);

typedef Struct_Adapter Mockery_Result_Struct_Adapter;
static class_info_entry_t mockery_result_stuct_adapter[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Point_Entry(1, Mockery_Result_Struct_Adapter, free, mockery_result_struct_custom_free),
    Init_Point_Entry(2, Mockery_Result_Struct_Adapter, to_json, mockery_result_struct_custom_to_json),
    Init_End___Entry(3, Mockery_Result_Struct_Adapter),
};
REGISTER_CLASS(Mockery_Result_Struct_Adapter, mockery_result_stuct_adapter);