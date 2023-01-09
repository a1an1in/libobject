/**
 * @file ZCompress.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-01-09
 */

#include "ZCompress.h"

 static int __deflate_buf(ZCompress *c, char *in, int in_len, char *out, int *out_len)
 {
    RETURN_IF(compressBound(in_len) < out_len, -1);
    return TRY_EXEC(compress(out, out_len, in, in_len));
 }

 static int __inflate_buf(ZCompress *c, char *in, int in_len, char *out, int *out_len)
 {
    return TRY_EXEC(uncompress(out, out_len, in, in_len));
 }

 static int __deflate_file(ZCompress *c, char *file_in, char *file_out)
 {
    return -1;
 }

 static int __inflate_file(ZCompress *c, char *file_in, char *file_out)
 {
    return -1;
 }

static class_info_entry_t zcompress_class_info[] = {
    Init_Obj___Entry(0, Compress, parent),
    Init_Nfunc_Entry(1, ZCompress, construct, NULL),
    Init_Nfunc_Entry(2, ZCompress, deconstruct, NULL),
    Init_Vfunc_Entry(3, ZCompress, deflate_buf, __deflate_buf),
    Init_Vfunc_Entry(4, ZCompress, inflate_buf, __inflate_buf),
    Init_Vfunc_Entry(5, ZCompress, deflate_file, __deflate_file),
    Init_Vfunc_Entry(6, ZCompress, inflate_file, __inflate_file),
    Init_End___Entry(7, ZCompress),
};
REGISTER_CLASS("ZCompress", zcompress_class_info);

