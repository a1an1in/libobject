#ifndef __ZCompress_H__
#define __ZCompress_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/compress/Compress.h>

typedef struct ZCompress_s ZCompress;

struct ZCompress_s{
    Compress parent;

    int (*construct)(ZCompress *,char *);
    int (*deconstruct)(ZCompress *);

    /*virtual methods reimplement*/
    int (*compress_buf)(ZCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*uncompress_buf)(ZCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*compress_file)(ZCompress *c, char *file_in, char *file_out);
    int (*uncompress_file)(ZCompress *c, char *file_in, char *file_out);
};

#endif
