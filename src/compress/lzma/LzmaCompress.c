/**
 * @file LzmaCompress.c
 * @Synopsis
 *    只是加入了模板还未调试
 * @author alan lin
 * @version 
 * @date 2023-01-09
 */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libobject/core/io/File.h>
#include "LzmaCompress.h"

 static int __compress_buf(LzmaCompress *c, char *in, int in_len, char *out, int *out_len)
 {
    int ret;

    TRY {
    } CATCH (ret) {}

    return ret;
 }

 static int __uncompress_buf(LzmaCompress *c, char *in, int in_len, char *out, int *out_len)
 {
    return 1;
 }

 static int __compress_file(LzmaCompress *c, char *file_in, char *file_out)
 {
    return -1;
 }

 static int __uncompress_file(LzmaCompress *c, char *file_name_in, char *file_name_out)
 {
    int ret, in_size, out_size;
    File *in_file, *out_file;
    allocator_t *allocator = c->parent.parent.allocator;
    char *in_buffer, *out_buffer;

    TRY {
      in_file = object_new(allocator, "File", NULL);
      out_file = object_new(allocator, "File", NULL);

      in_file->open(in_file, file_name_in, "r+");
      in_size = in_file->get_size(in_file);
      dbg_str(DBG_VIP, "uncompress_file, file size:%d", in_size);

    } CATCH (ret) {} FINALLY {
      object_destroy(in_file);
      object_destroy(out_file);
    }

    return ret;
 }

static class_info_entry_t zcompress_class_info[] = {
    Init_Obj___Entry(0, Compress, parent),
    Init_Nfunc_Entry(1, LzmaCompress, construct, NULL),
    Init_Nfunc_Entry(2, LzmaCompress, deconstruct, NULL),
    Init_Vfunc_Entry(3, LzmaCompress, compress_buf, __compress_buf),
    Init_Vfunc_Entry(4, LzmaCompress, uncompress_buf, __uncompress_buf),
    Init_Vfunc_Entry(5, LzmaCompress, compress_file, __compress_file),
    Init_Vfunc_Entry(6, LzmaCompress, uncompress_file, __uncompress_file),
    Init_End___Entry(7, LzmaCompress),
};
REGISTER_CLASS("LzmaCompress", zcompress_class_info);

