#ifndef __DeflateCompress_H__
#define __DeflateCompress_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/compress/Compress.h>

typedef struct DeflateCompress_s DeflateCompress;

struct DeflateCompress_s{
    Compress parent;

    int (*construct)(DeflateCompress *,char *);
    int (*deconstruct)(DeflateCompress *);

    /*virtual methods reimplement*/
    int (*compress_buf)(DeflateCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*uncompress_buf)(DeflateCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*compress_file)(DeflateCompress *c, char *file_in, char *file_out);
    int (*uncompress_file)(DeflateCompress *c, char *file_in, char *file_out);
    int (*compress)(DeflateCompress *c, File *in, long in_len, File *out, long *out_len);
    int (*uncompress)(DeflateCompress *c, File *in, long in_len, File *out, long *out_len);
};

#endif
