/**
 * @file GZCompress.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-10-07
 */
#ifndef GZIP_H
#define GZIP_H

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libobject/core/io/File.h>
#include "zlib.h"
#include "GZCompress.h"

/* ref to https://blog.csdn.net/chary8088/article/details/48047835/ */

 static int __compress_buf(GZCompress *c, char *in, int in_len, char *out, int *out_len)
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

int __uncompress_buf(GZCompress *c, char *in, int in_len, char *out, int *out_len)
{
    int err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    static char dummy_head[2] = {
        0x8 + 0x7 * 0x10,
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
    };
    d_stream.zalloc = NULL;
    d_stream.zfree = NULL;
    d_stream.opaque = NULL;
    d_stream.next_in  = in;
    d_stream.avail_in = 0;
    d_stream.next_out = out;

    //只有设置为MAX_WBITS + 16才能在解压带header和trailer的文本
    if (inflateInit2(&d_stream, MAX_WBITS + 16) != Z_OK) return -1;

    while (d_stream.total_out < *out_len && d_stream.total_in < in_len) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        err = inflate(&d_stream, Z_NO_FLUSH);
    
        if (err == Z_STREAM_END) break;
        else if (err == Z_OK) continue;
        else if (err == Z_DATA_ERROR) {
            d_stream.next_in = (Bytef*) dummy_head;
            d_stream.avail_in = sizeof(dummy_head);
            if ((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) {return -1;}
        } else return -1;
    }

    if (inflateEnd(&d_stream) != Z_OK) return -1;
    *out_len = d_stream.total_out;
    return 0;
}

 static int __compress_file(GZCompress *c, char *file_in, char *file_out)
 {
    return -1;
 }

 static int __uncompress_file(GZCompress *c, char *file_name_in, char *file_name_out)
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
    Init_Nfunc_Entry(1, GZCompress, construct, NULL),
    Init_Nfunc_Entry(2, GZCompress, deconstruct, NULL),
    Init_Vfunc_Entry(3, GZCompress, compress_buf, __compress_buf),
    Init_Vfunc_Entry(4, GZCompress, uncompress_buf, __uncompress_buf),
    Init_Vfunc_Entry(5, GZCompress, compress_file, __compress_file),
    Init_Vfunc_Entry(6, GZCompress, uncompress_file, __uncompress_file),
    Init_End___Entry(7, GZCompress),
};
REGISTER_CLASS("GZCompress", zcompress_class_info);
#endif // GZIP_H

