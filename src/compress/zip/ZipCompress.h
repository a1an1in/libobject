#ifndef __ZipCompress_H__
#define __ZipCompress_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/compress/Compress.h>

typedef struct ZipCompress_s ZipCompress;

struct ZipCompress_s{
    Compress parent;

    int (*construct)(ZipCompress *,char *);
    int (*deconstruct)(ZipCompress *);

    /*virtual methods reimplement*/
    int (*compress_buf)(ZipCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*uncompress_buf)(ZipCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*compress_file)(ZipCompress *c, char *file_in, char *file_out);
    int (*uncompress_file)(ZipCompress *c, char *file_in, char *file_out);
};

#endif
