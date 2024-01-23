#ifndef __Bz2Compress_H__
#define __Bz2Compress_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/compress/Compress.h>

typedef struct Bz2Compress_s Bz2Compress;

struct Bz2Compress_s{
    Compress parent;

    int (*construct)(Bz2Compress *,char *);
    int (*deconstruct)(Bz2Compress *);

    /*virtual methods reimplement*/
    int (*compress_buf)(Bz2Compress *c, char *in, int in_len, char *out, int *out_len);
    int (*uncompress_buf)(Bz2Compress *c, char *in, int in_len, char *out, int *out_len);
    int (*compress_file)(Bz2Compress *c, char *file_in, char *file_out);
    int (*uncompress_file)(Bz2Compress *c, char *file_in, char *file_out);
};

#endif
