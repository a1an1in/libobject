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

static int __run_func(FShell *shell, String *str)
{
    int ret, i, cnt, len;
    char *arg, *func_name;
    fshell_func_t func = NULL;
    Map *map = shell->variable_map;
    node_malloc_variable_info_t *info = NULL;
    void *par[20] = {0};
    char *func_str[128];

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
            /* 参数加引号为表示字符串， 不加引号为数字或者变量名 */
            if (arg[0] == '"') {
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

        if (strcmp(func_name, "fsh_add_stub_hooks") == 0) {
            ret = func(shell, par[0], par[1], par[2], par[3], par[4],
                       par[5], par[6], par[7], par[8], par[9], 
                       par[10], par[11], par[12], par[13], par[14],
                       par[15], par[16], par[17], par[18]);
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
REGISTER_CLASS("FShell", shell_class_info);

