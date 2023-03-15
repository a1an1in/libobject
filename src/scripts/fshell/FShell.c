/**
 * @file FShell.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <stdlib.h>
#include <signal.h>
#include <libobject/core/utils/string.h>
#include <libobject/scripts/FShell.h>

static void __close_callback(int fd, short event_res, void *arg)
{
    FShell *shell = (FShell *)arg;

    shell->close_flag = 1;
    dbg_str(DBG_SUC, "close shel");
    event_del(shell->signal);

}

static int __construct(FShell *shell, char *init_str)
{
    Map *map;
    int ev_fd;
    struct event *signal;
    struct event_base* base = event_base_get_default_instance();
    allocator_t *allocator = shell->parent.allocator;

    map = object_new(shell->parent.allocator, "RBTree_Map", NULL);
    map->set_cmp_func(map, string_key_cmp_func);
    shell->map = map;

    signal = allocator_mem_alloc(allocator, sizeof(struct event));
    if (signal == NULL) {
        return -1;
    }
    shell->signal = signal;
    shell->close_flag = 0;

#if (!defined(WINDOWS_USER_MODE))
    ev_fd = SIGINT;
#else
    ev_fd = SIGINT;
#endif

    dbg_str(DBG_SUC, "fshell event base:%p", base);
    event_assign(signal, base, ev_fd, EV_SIGNAL|EV_PERSIST,
                 __close_callback, shell);

    event_add(signal, NULL);

    sprintf(shell->prompt, "%s", "fshell$ ");

    return 0;
}

static int __deconstruct(FShell *shell)
{
    allocator_t *allocator = shell->parent.allocator;

    object_destroy(shell->map);
    event_del(shell->signal);
    allocator_mem_free(allocator, shell->signal);

    return 0;
}

static int __set_prompt(FShell *shell, char *prompt)
{
}

static int __run_func(FShell *shell, String *str)
{
    int ret, i, cnt;
    char *arg;
    fshell_func_t func = NULL;
    void *par[20] = {0};

    TRY {
        THROW_IF(str == NULL, -1);
        cnt = str->split(str, "[,\t\n();]", -1);

        THROW_IF(cnt <= 0, 0);
        arg = str->get_splited_cstr(str, 0);
        EXEC(shell->get_func_addr(shell, NULL, arg, &func));
        THROW_IF(func == NULL, -1);

        for (i = 1; i < cnt; i++) {
            dbg_str(DBG_DETAIL, "run at here, cnt:%d", cnt);
            arg = str->get_splited_cstr(str, i);
            if (arg != NULL) {
                dbg_str(DBG_DETAIL, "%d:%s", i, arg);
            }
            arg = str_trim(arg);
            if (arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X')) {
                par[i - 1] = str_hex_to_int(arg);
                dbg_str(DBG_DETAIL, "par i:%d value:%x", i - 1, par[i - 1]);
            } else {
                par[i - 1] = arg;
            }
        }
        ret = func(par[0], par[1], par[2], par[3], par[4],
                par[5], par[6], par[7], par[8], par[9], 
                par[10], par[11], par[12], par[13], par[14],
                par[15], par[16], par[17], par[18], par[19]);
        dbg_str(DBG_DETAIL, "run func ret:%d", ret);
        
    } CATCH (ret) {
    }

    return ret;
}

static int __is_statement(FShell *shell, char *str)
{
    int len, ret = 0;

    TRY {
        len = strlen(str);
        THROW_IF(len <= 1, -1);
        if (str[len - 1] == ';') {
            return 1;
        }
    } CATCH (ret) {
    }

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
    Init_End___Entry(11, FShell),
};
REGISTER_CLASS("FShell", shell_class_info);

int test_hello()
{
    printf("hello world\n");
    return 0;
}

int test_add(int a, int b)
{
    printf("test add, a:%d, b:%d, count:%d\n", a, b, a + b);
    return a + b;
}

int test_printf(char *fmt, ...)
{
    va_list ap;

    printf("test_printf, fmt:%s\n", fmt);
    dbg_buf(DBG_DETAIL, "fmt:", fmt, strlen(fmt));
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    return 1;
}

