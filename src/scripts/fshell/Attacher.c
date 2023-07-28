/**
 * @file Attacher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-07-28
 */

#include <libobject/scripts/fshell/Attacher.h>

static int __construct(Attacher *attacher, char *init_str)
{
    return 0;
}

static int __deconstruct(Attacher *attacher)
{
    return 0;
}

static class_info_entry_t attacher_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Attacher, construct, __construct),
    Init_Nfunc_Entry(2, Attacher, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Attacher, attach, NULL),
    Init_Vfunc_Entry(4, Attacher, detach, NULL),
    Init_Vfunc_Entry(5, Attacher, call, NULL),
    Init_Vfunc_Entry(6, Attacher, add_lib, NULL),
    Init_Vfunc_Entry(7, Attacher, remove_lib, NULL),
    Init_End___Entry(8, Attacher),
};
REGISTER_CLASS("Attacher", attacher_class_info);

