/**
 * @file Attacher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-07-28
 */

#include <libobject/attacher/Attacher.h>

static int __construct(Attacher *attacher, char *init_str)
{
    attacher->map = object_new(attacher->parent.allocator, "RBTree_Map", NULL);
    return 0;
}

static int __deconstrcut(Attacher *attacher)
{
    Map *map = (Map *)attacher->map;

    object_destroy(map);

    return 0;
}

static class_info_entry_t attacher_class_info[] = {
    Init_Obj___Entry( 0, Obj, parent),
    Init_Nfunc_Entry( 1, Attacher, construct, __construct),
    Init_Nfunc_Entry( 2, Attacher, deconstruct, __deconstrcut),
    Init_Vfunc_Entry( 3, Attacher, attach, NULL),
    Init_Vfunc_Entry( 4, Attacher, detach, NULL),
    Init_Vfunc_Entry( 5, Attacher, get_function_address, NULL),
    Init_Vfunc_Entry( 6, Attacher, read, NULL),
    Init_Vfunc_Entry( 7, Attacher, write, NULL),
    Init_Vfunc_Entry( 8, Attacher, malloc, NULL),
    Init_Vfunc_Entry( 9, Attacher, free, NULL),
    Init_Vfunc_Entry(10, Attacher, call_address_with_value_pars, NULL),
    Init_Vfunc_Entry(11, Attacher, call_address, NULL),
    Init_Vfunc_Entry(12, Attacher, call_from_lib, NULL),
    Init_Vfunc_Entry(13, Attacher, add_lib, NULL),
    Init_Vfunc_Entry(14, Attacher, remove_lib, NULL),
    Init_End___Entry(15, Attacher),
};
REGISTER_CLASS("Attacher", attacher_class_info);

