#ifndef __LzmaCompress_H__
#define __LzmaCompress_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/compress/Compress.h>

typedef struct LzmaCompress_s LzmaCompress;

struct LzmaCompress_s{
    Compress parent;

    int (*construct)(LzmaCompress *,char *);
    int (*deconstruct)(LzmaCompress *);

    /*virtual methods reimplement*/
    int (*compress_buf)(LzmaCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*uncompress_buf)(LzmaCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*compress_file)(LzmaCompress *c, char *file_in, char *file_out);
    int (*uncompress_file)(LzmaCompress *c, char *file_in, char *file_out);
};

#endif
