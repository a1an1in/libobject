/**
 * @file FShell.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/scripts/FShell.h>

static int __construct(FShell *shell, char *init_str)
{
    Map *map;

    map = object_new(shell->parent.allocator, "RBTree_Map", NULL);
    map->set_cmp_func(map, string_key_cmp_func);
    shell->map = map;

    return 0;
}

static int __deconstruct(FShell *shell)
{
    object_destroy(shell->map);

    return 0;
}
static class_info_entry_t shell_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, FShell, construct, __construct),
    Init_Nfunc_Entry(2, FShell, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, FShell, load, NULL),
    Init_Vfunc_Entry(4, FShell, unload, NULL),
    Init_Vfunc_Entry(5, FShell, get_func_addr, NULL),
    Init_Vfunc_Entry(6, FShell, get_func_name, NULL),
    Init_End___Entry(7, FShell),
};
REGISTER_CLASS("FShell", shell_class_info);

