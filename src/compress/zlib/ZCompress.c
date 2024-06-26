/**
 * @file ZCompress.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-01-09
 */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libobject/core/io/File.h>
#include "ZCompress.h"

 static int __compress_buf(ZCompress *c, char *in, int in_len, char *out, int *out_len)
 {
    int ret;

    TRY {
      THROW_IF(compressBound(in_len) > *out_len, -1);
      EXEC(compress(out, out_len, in, in_len));

    } CATCH (ret) {
       dbg_str(DBG_ERROR, "in_len:%d, out_len:%d, compressBound(in_len):%d", 
               in_len, *out_len, compressBound(in_len));
    }

    return ret;
 }

 static int __uncompress_buf(ZCompress *c, char *in, int in_len, char *out, int *out_len)
 {
    return TRY_EXEC(uncompress(out, out_len, in, in_len));
 }

 static int __compress_file(ZCompress *c, char *file_in, char *file_out)
 {
    return -1;
 }

 static int __uncompress_file(ZCompress *c, char *file_name_in, char *file_name_out)
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

      in_buffer = allocator_mem_alloc(allocator, in_size);
      out_buffer = allocator_mem_alloc(allocator, in_size * 2);

      in_file->read(in_file, in_buffer, in_size);
      c->uncompress_buf(c, in_buffer, in_size, out_buffer, &out_size);
      
      out_file->open(out_file, file_name_out, "w+");
      out_file->write(out_file, out_buffer, out_size);
    } CATCH (ret) {} FINALLY {
      allocator_mem_free(allocator, in_buffer);
      allocator_mem_free(allocator, out_buffer);
      object_destroy(in_file);
      object_destroy(out_file);
    }

    return ret;
 }

static class_info_entry_t zcompress_class_info[] = {
    Init_Obj___Entry(0, Compress, parent),
    Init_Nfunc_Entry(1, ZCompress, construct, NULL),
    Init_Nfunc_Entry(2, ZCompress, deconstruct, NULL),
    Init_Vfunc_Entry(3, ZCompress, compress_buf, __compress_buf),
    Init_Vfunc_Entry(4, ZCompress, uncompress_buf, __uncompress_buf),
    Init_Vfunc_Entry(5, ZCompress, compress_file, __compress_file),
    Init_Vfunc_Entry(6, ZCompress, uncompress_file, __uncompress_file),
    Init_End___Entry(7, ZCompress),
};
REGISTER_CLASS(ZCompress, zcompress_class_info);

