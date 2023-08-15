/**
 * @file Attacher.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-07-28
 */

#include <libobject/scripts/fshell/Attacher.h>

static class_info_entry_t attacher_class_info[] = {
    Init_Obj___Entry( 0, Obj, parent),
    Init_Nfunc_Entry( 1, Attacher, construct, NULL),
    Init_Nfunc_Entry( 2, Attacher, deconstruct, NULL),
    Init_Vfunc_Entry( 3, Attacher, attach, NULL),
    Init_Vfunc_Entry( 4, Attacher, detach, NULL),
    Init_Vfunc_Entry( 5, Attacher, get_function_address, NULL),
    Init_Vfunc_Entry( 6, Attacher, read, NULL),
    Init_Vfunc_Entry( 7, Attacher, write, NULL),
    Init_Vfunc_Entry( 8, Attacher, call, NULL),
    Init_Vfunc_Entry( 9, Attacher, add_lib, NULL),
    Init_Vfunc_Entry(10, Attacher, remove_lib, NULL),
    Init_End___Entry(11, Attacher),
};
REGISTER_CLASS("Attacher", attacher_class_info);

