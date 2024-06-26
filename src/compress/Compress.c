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
    Init_Vfunc_Entry(3, Compress, compress_buf, NULL),
    Init_Vfunc_Entry(4, Compress, uncompress_buf, NULL),
    Init_Vfunc_Entry(5, Compress, compress_file, NULL),
    Init_Vfunc_Entry(6, Compress, uncompress_file, NULL),
    Init_Vfunc_Entry(7, Compress, compress, NULL),
    Init_Vfunc_Entry(8, Compress, uncompress, NULL),
    Init_End___Entry(9, Compress),
};
REGISTER_CLASS(Compress, compress_class_info);

