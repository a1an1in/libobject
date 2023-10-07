#ifndef __GZCompress_H__
#define __GZCompress_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/compress/Compress.h>

typedef struct GZCompress_s GZCompress;

struct GZCompress_s{
    Compress parent;

    int (*construct)(GZCompress *,char *);
    int (*deconstruct)(GZCompress *);

    /*virtual methods reimplement*/
    int (*compress_buf)(GZCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*uncompress_buf)(GZCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*compress_file)(GZCompress *c, char *file_in, char *file_out);
    int (*uncompress_file)(GZCompress *c, char *file_in, char *file_out);
};

#endif
