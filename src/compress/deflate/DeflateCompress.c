/**
 * @file DeflateCompress.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-01-09
 */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libobject/core/io/File.h>
#include "zlib.h"
#include "DeflateCompress.h"

extern int deflate_uncompress(FILE *source, long in_len, FILE *dest, long *out_len);
extern int deflate_compress(FILE *source, long in_len, FILE *dest, long *out_len);

static int __compress_buf(DeflateCompress *c, char *in, int in_len, char *out, int *out_len)
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
                          Z_DEFLATED, -MAX_WBITS, 8, 
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

static int __uncompress_buf(DeflateCompress *c, char *in, int in_len, char *out, int *out_len)
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
    if (inflateInit2(&d_stream, -MAX_WBITS) != Z_OK) return -1;

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

 static int __compress(DeflateCompress *c, File *in, long in_len, File *out, long *out_len)
 {
    dbg_str(DBG_SUC, "zip compress, in len:%d", in_len);
    return TRY_EXEC(deflate_compress(in->f, in_len + 1, out->f, out_len));
 }

 static int __uncompress(DeflateCompress *c, File *in, long in_len, File *out, long *out_len)
 {
    return TRY_EXEC(deflate_uncompress(in->f, in_len, out->f, out_len));
 }

static class_info_entry_t zcompress_class_info[] = {
    Init_Obj___Entry(0, Compress, parent),
    Init_Nfunc_Entry(1, DeflateCompress, construct, NULL),
    Init_Nfunc_Entry(2, DeflateCompress, deconstruct, NULL),
    Init_Vfunc_Entry(3, DeflateCompress, compress_buf, __compress_buf),
    Init_Vfunc_Entry(4, DeflateCompress, uncompress_buf, __uncompress_buf),
    Init_Vfunc_Entry(5, DeflateCompress, compress, __compress),
    Init_Vfunc_Entry(6, DeflateCompress, uncompress, __uncompress),
    Init_End___Entry(7, DeflateCompress),
};
REGISTER_CLASS("DeflateCompress", zcompress_class_info);