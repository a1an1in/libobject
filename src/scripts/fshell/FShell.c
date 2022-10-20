/**
 * @file FShell.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/scripts/FShell.h>

static void __close_callback(int fd, short event_res, void *arg)
{
    FShell *shell = (FShell *)arg;

    shell->close_flag = 1;
    dbg_str(DBG_SUC, "close shel");

}

static int __construct(FShell *shell, char *init_str)
{
    Map *map;
    int ev_fd;
    struct event *signal;
    struct event_base* base = get_default_event_base();
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

static class_info_entry_t shell_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, FShell, construct, __construct),
    Init_Nfunc_Entry(2, FShell, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, FShell, load, NULL),
    Init_Vfunc_Entry(4, FShell, unload, NULL),
    Init_Vfunc_Entry(5, FShell, get_func_addr, NULL),
    Init_Vfunc_Entry(6, FShell, get_func_name, NULL),
    Init_Vfunc_Entry(7, FShell, open, NULL),
    Init_Vfunc_Entry(8, FShell, set_prompt, __set_prompt),
    Init_End___Entry(9, FShell),
};
REGISTER_CLASS("FShell", shell_class_info);

