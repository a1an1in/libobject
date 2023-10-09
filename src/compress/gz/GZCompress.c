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

extern int gz_compress(FILE *source, FILE *dest);
extern int gz_uncompress(FILE *source, FILE *dest);

/* ref to https://blog.csdn.net/chary8088/article/details/48047835/ */
/* 因为不能把文件分段压缩， 所以这个buf压缩接口其实就是压缩整个文件了， 这个
 * 函数内部有没有对buffer大小做分片除了， 所以就不使用这个接口， 保留下来以供
 * 后面学习使用， 接口使用是没有问题的， 但是不建议使用，请直接使用compress_file。
 **/
static int __compress_buf(GZCompress *c, char *in, int in_len, char *out, int *out_len)
{
  int err = 0, ret;
  z_stream c_stream = {0};

  TRY {
    THROW_IF(in == NULL || in_len <= 0, -1);
    c_stream.next_in = in;
    c_stream.avail_in = in_len;
    c_stream.next_out = out;
    c_stream.avail_out = *out_len;

    // use parm 'MAX_WBITS+16' so that gzip headers are contained
    THROW_IF(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, 
                          Z_DEFLATED, MAX_WBITS + 16, 8, 
                          Z_DEFAULT_STRATEGY) != Z_OK, -1);
    
    while (c_stream.avail_in != 0 && c_stream.total_out < *out_len) {
      THROW_IF(deflate(&c_stream, Z_NO_FLUSH) != Z_OK, -1);
    }

    THROW_IF(c_stream.avail_in != 0, -1);

    for (; ;) {
      if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
      THROW_IF(err != Z_OK, -1);
    }

    THROW_IF(deflateEnd(&c_stream) != Z_OK, -1);

    *out_len = c_stream.total_out;
  } CATCH(ret) {} FINALLY {}

  return ret;
}

static int __uncompress_buf(GZCompress *c, char *in, int in_len, char *out, int *out_len)
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
    return 1;
}

 static int __compress_file(GZCompress *c, char *file_in, char *file_out)
 {
    File *in_file, *out_file;
    allocator_t *allocator = c->parent.parent.allocator;
    int ret;

    TRY {
      in_file = object_new(allocator, "File", NULL);
      out_file = object_new(allocator, "File", NULL);
      in_file->open(in_file, file_in, "r+");
      out_file->open(out_file, file_out, "w+");

      EXEC(gz_compress(in_file->f, out_file->f));
    } CATCH (ret) {} FINALLY {
      object_destroy(in_file);
      object_destroy(out_file);
    }

    return ret;
 }

 static int __uncompress_file(GZCompress *c, char *file_in, char *file_out)
 {
    File *in_file, *out_file;
    allocator_t *allocator = c->parent.parent.allocator;
    int ret;

    TRY {
      in_file = object_new(allocator, "File", NULL);
      out_file = object_new(allocator, "File", NULL);
      in_file->open(in_file, file_in, "r+");
      out_file->open(out_file, file_out, "w+");
    
      EXEC(gz_uncompress(in_file->f, out_file->f));
    } CATCH (ret) {} FINALLY {
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

