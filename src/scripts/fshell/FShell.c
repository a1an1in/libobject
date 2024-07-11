/**
 * @file FShell.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2022-08-20
 */

#include <stdlib.h>
#include <signal.h>
#include <libobject/core/utils/string.h>
#include <libobject/concurrent/Worker.h>
#include <libobject/scripts/fshell/FShell.h>
#include <libobject/scripts/fshell/api.h>

FShell *g_shell;

static void __close_fshell_callback(void *arg)
{
    FShell *shell = (FShell *)arg;

    shell->close_flag = 1;
    dbg_str(DBG_VIP, "close shel");

}

static int __construct(FShell *shell, char *init_str)
{
    Map *map;
    struct event *signal;
    int trustee_flag = 1;
    int value_type = VALUE_TYPE_STRUCT_POINTER;
    allocator_t *allocator = shell->parent.allocator;

    map = object_new(shell->parent.allocator, "RBTree_Map", NULL);
    map->set_cmp_func(map, string_key_cmp_func);
    shell->map = map;
    shell->close_flag = 0;

    map = object_new(allocator, "RBTree_Map", NULL);
    map->set_cmp_func(map, string_key_cmp_func);
    map->set(map, "/Map/trustee_flag", &trustee_flag);
    map->set(map, "/Map/value_type", &value_type);
    map->set(map, "/Map/class_name", "Fsh_Variable_Info");
    shell->variable_map = map;

    sprintf(shell->prompt, "%s", "fshell$ ");

    return 0;
}

static int __deconstruct(FShell *shell)
{
    allocator_t *allocator = shell->parent.allocator;

    dbg_str(DBG_DETAIL, "fshell deconstruct in, shell->worker=%p", shell->worker);
    object_destroy(shell->variable_map);
    object_destroy(shell->map);
    worker_destroy(shell->worker);
    dbg_str(DBG_DETAIL, "fshell deconstruct out");

    return 0;
}

static int __init(FShell *shell)
{
    int ev_fd;
    struct event *signal;
    struct event_base* base = event_base_get_default_instance();
    allocator_t *allocator = shell->parent.allocator;

    if (g_shell != NULL) return -1;

#if (!defined(WINDOWS_USER_MODE))
    ev_fd = SIGINT;
#else
    ev_fd = SIGINT;
#endif

    shell->worker = signal_worker(allocator, ev_fd, __close_fshell_callback, shell);
    g_shell = shell;
    dbg_str(DBG_DETAIL, "fshell init, shell->worker=%p", shell->worker);

    return 0;
}

static int __set_prompt(FShell *shell, char *prompt)
{
    memset(shell->prompt, 0, sizeof(shell->prompt));
    sprintf(shell->prompt, "%s", prompt);
    return 0;
}

static int __convert_object_method_paramaters(FShell *shell, String *str)
{
    int ret, start, len;
    char *regex = "(#[a-z0-9A-Z._-]+)";
    char variable_name[32];

    TRY {
        EXEC(str->get_substring(str, regex, 0, &start, &len));
        strncpy(variable_name, STR2A(str) + start, len);
        str->replace(str, variable_name, "0x123", 1);
    } CATCH (ret) { } {}

    return ret;
}

static int __run_func(FShell *shell, String *str)
{
    int ret, i, cnt, len;
    char *arg, *func_name;
    fshell_func_t func = NULL;
    Map *map = shell->variable_map;
    fsh_malloc_variable_info_t *info = NULL;
    void *par[20] = {0};
    char func_str[128];

    TRY {
        THROW_IF(str == NULL, -1);
        strcpy(func_str, STR2A(str));
        cnt = str->split(str, "[,\t\n();]", -1);

        THROW_IF(cnt <= 0, 0);
        func_name = str->get_splited_cstr(str, 0);
        EXEC(shell->get_func_addr(shell, NULL, func_name, &func));
        THROW_IF(func == NULL, -1);

        for (i = 1; i < cnt; i++) {
            dbg_str(DBG_DETAIL, "run at here, cnt:%d", cnt);
            arg = str->get_splited_cstr(str, i);
            if (arg != NULL) {
                dbg_str(DBG_DETAIL, "%d:%s", i, arg);
            }
            arg = str_trim(arg);

            if (arg[0] == '"')
            /* 参数加引号为表示字符串， 不加引号为数字或者变量名 */
            {
                len = strlen(arg);
                THROW_IF(arg[len - 1] != '"', -1);
                arg[len - 1] = '\0';
                par[i - 1] = arg + 1;
            } else if (arg[0] == '#')
            /* #表示这是一个变量的地址，需要从变量表里根据变量名查询得到地址, 
             * shell会解析$符号， 所以使用#。
             */
            {
                map->search(map, arg, &info);
                THROW_IF(info == NULL, -1);
                par[i - 1] = info->addr;
                dbg_str(DBG_INFO, "run_func, transfer variable %s address %p", arg, info->addr);
            } else if (arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X')) {
                par[i - 1] = str_hex_to_integer(arg);
                dbg_str(DBG_DETAIL, "par i:%d value:%x", i - 1, par[i - 1]);
            } else if (arg[0] == '0' && (arg[1] == 'd' || arg[1] == 'D')) {
                par[i - 1] = atoi(arg + 2);
            } else {
                par[i - 1] = atoi(arg);
            }
        }

        if (strncmp(func_name, "fsh_", strlen("fsh_")) == 0)
        /* fsh_ 打头的函数， 第一个参数为shell， 其余的参数往后移 */
        {
            ret = func(shell, par[0], par[1], par[2], par[3], par[4],
                       par[5], par[6], par[7], par[8], par[9], 
                       par[10], par[11], par[12], par[13], par[14],
                       par[15], par[16], par[17], par[18]);
        } else if (strncmp(func_name, "object_call_method", strlen("object_call_method")) == 0)
        /* 调用object method格式为method_name<arg1:arg2:arg3:...> */
        {
            char method[64];
            strcpy(method, par[1]);
            str->assign(str, method);
            EXEC(__convert_object_method_paramaters(shell, str));
            dbg_str(DBG_SUC, "method:%s", STR2A(str));
        } else {
            ret = func(par[0], par[1], par[2], par[3], par[4],
                       par[5], par[6], par[7], par[8], par[9], 
                       par[10], par[11], par[12], par[13], par[14],
                       par[15], par[16], par[17], par[18], par[19]);
        }

        dbg_str(DBG_VIP, "fshell execute:%s, ret:%d", func_str, ret);
        THROW(ret);
    } CATCH (ret) { }

    return ret;
}

static int __is_statement(FShell *shell, char *str)
{
    int len, ret = 0;

    TRY {
        len = strlen(str);
        THROW_IF(len <= 1, 0);
        if (str[len - 2] == ';') {
            return 1;
        } else {
            return 0;
        }
    } CATCH (ret) { }

    return ret;
}

static class_info_entry_t shell_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , FShell, construct, __construct),
    Init_Nfunc_Entry(2 , FShell, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , FShell, load, NULL),
    Init_Vfunc_Entry(4 , FShell, unload, NULL),
    Init_Vfunc_Entry(5 , FShell, get_func_addr, NULL),
    Init_Vfunc_Entry(6 , FShell, get_func_name, NULL),
    Init_Vfunc_Entry(7 , FShell, open_ui, NULL),
    Init_Vfunc_Entry(8 , FShell, set_prompt, __set_prompt),
    Init_Vfunc_Entry(9 , FShell, run_func, __run_func),
    Init_Vfunc_Entry(10, FShell, is_statement, __is_statement),
    Init_Vfunc_Entry(11, FShell, init, __init),
    Init_End___Entry(12, FShell),
};
REGISTER_CLASS(FShell, shell_class_info);

int fsh_variable_info_new(allocator_t *allocator, cjson_t *c, void **value)
{
    fsh_malloc_variable_info_t *v;
    int ret;

    TRY {
        v = allocator_mem_alloc(allocator, sizeof(fsh_malloc_variable_info_t));
        *value = v;
        while (c) {
            // if (strcmp(c->string, "size") == 0) {
            //     v->st.st_size = c->valueint;
            // } else if (strcmp(c->string, "file_name") == 0) {
            //     v->file_name = allocator_mem_alloc(allocator, strlen(c->valuestring) + 1);
            //     strcpy(v->file_name, c->valuestring);
            // } else {
            //     dbg_str(DBG_VIP, "wrong value name:%s", c->string);
            // }

            c = c->next;
        }
    } CATCH (ret) {}

    return ret;
}

int fsh_variable_info_alloc(allocator_t *allocator, uint32_t value_type, char *class_name, uint32_t size, char *name, void **value)
{
    fsh_malloc_variable_info_t *info;
    int ret;

    TRY {
        switch (value_type) {
            case VALUE_TYPE_ALLOC_POINTER: {
                info = allocator_mem_alloc(allocator, sizeof(fsh_malloc_variable_info_t) + size);
                info->value_type = VALUE_TYPE_ALLOC_POINTER;
                info->addr = info->value; //记录分配给用户使用的地址。
                strcpy(info->name, name);
                break;
            }
            case VALUE_TYPE_STUB_POINTER:
                info = allocator_mem_alloc(allocator, sizeof(fsh_malloc_variable_info_t) + sizeof(void *));
                info->addr = stub_alloc();
                info->value_type = VALUE_TYPE_STUB_POINTER;
                strcpy(info->name, name);
                break;
            case VALUE_TYPE_OBJ_POINTER:
                info = allocator_mem_alloc(allocator, sizeof(fsh_malloc_variable_info_t) + sizeof(void *));
                info->addr = object_new(allocator, class_name, NULL);
                info->value_type = VALUE_TYPE_OBJ_POINTER;
                strcpy(info->name, name);
                break;
            case VALUE_TYPE_STRUCT_POINTER:
                break;
            default:
                break;
        }
        *value = info;

    } CATCH (ret) {}

    return ret;
}

int fsh_variable_info_free(allocator_t *allocator, fsh_malloc_variable_info_t *info)
{
    switch (info->value_type) {
        case VALUE_TYPE_ALLOC_POINTER: {
            dbg_str(DBG_VIP, "node_mfree alloc pointer, name:%s, addr:%p", info->name, info->value);
            allocator_mem_free(allocator, info);
            break;
        }
        case VALUE_TYPE_STUB_POINTER:
            dbg_str(DBG_VIP, "node_mfree stub, name:%s, addr:%p", info->name, info->addr);
            stub_free(info->addr);
            allocator_mem_free(allocator, info);
            break;
        case VALUE_TYPE_OBJ_POINTER:
            dbg_str(DBG_SUC, "node_mfree object, name:%s, addr:%p", info->name, info->addr);
            object_destroy(info->addr);
            allocator_mem_free(allocator, info);
        case VALUE_TYPE_STRUCT_POINTER:
            break;
        default:
            break;
    }

    return 1;
}

int fsh_variable_info_to_json(cjson_t *root, void *element)
{
    cjson_t *item = NULL;
    // fs_file_info_t *o = (fs_file_info_t *)element;

    // item = cjson_create_object();
    // cjson_add_string_to_object(item, "file_name", o->file_name);
    // cjson_add_number_to_object(item, "size", o->st.st_size);
    // if (item != NULL) {
    //     cjson_add_item_to_array(root, item);
    // }

    return 1;
}

int fsh_variable_info_print(int index, fsh_malloc_variable_info_t *info)
{
    dbg_str(DBG_INFO, "index:%d, file name:%s, value_type:%d, addr:%p", 
            index, info->name, info->value_type, info->addr);

    return 1;
}

static class_info_entry_t fsh_variable_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Point_Entry(1, Struct_Adapter, new, fsh_variable_info_new),
    Init_Point_Entry(2, Struct_Adapter, alloc, fsh_variable_info_alloc),
    Init_Point_Entry(3, Struct_Adapter, free, fsh_variable_info_free),
    Init_Point_Entry(4, Struct_Adapter, to_json, fsh_variable_info_to_json),
    Init_Point_Entry(5, Struct_Adapter, print, fsh_variable_info_print),
    Init_End___Entry(6, Struct_Adapter),
};
REGISTER_CLASS(Fsh_Variable_Info, fsh_variable_info);