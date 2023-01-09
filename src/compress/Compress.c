/**
 * @file Compress.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-01-09
 */

#include <libobject/compress/Compress.h>

static class_info_entry_t compress_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Compress, construct, NULL),
    Init_Nfunc_Entry(2, Compress, deconstruct, NULL),
    Init_Vfunc_Entry(3, Compress, deflate_buf, NULL),
    Init_Vfunc_Entry(4, Compress, inflate_buf, NULL),
    Init_Vfunc_Entry(5, Compress, deflate_file, NULL),
    Init_Vfunc_Entry(6, Compress, inflate_file, NULL),
    Init_End___Entry(7, Compress),
};
REGISTER_CLASS("Compress", compress_class_info);

