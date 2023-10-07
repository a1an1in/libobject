#ifndef __Compress_H__
#define __Compress_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Compress_s Compress;

struct Compress_s{
    Obj parent;

    int (*construct)(Compress *,char *);
    int (*deconstruct)(Compress *);

    /*virtual methods reimplement*/
    int (*compress_buf)(Compress *c, char *in, int in_len, char *out, int *out_len);
    int (*uncompress_buf)(Compress *c, char *in, int in_len, char *out, int *out_len);
    int (*compress_file)(Compress *c, char *file_in, char *file_out);
    int (*uncompress_file)(Compress *c, char *file_in, char *file_out);
};

#endif
