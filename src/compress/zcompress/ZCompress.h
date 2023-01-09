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
    int (*deflate_buf)(ZCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*inflate_buf)(ZCompress *c, char *in, int in_len, char *out, int *out_len);
    int (*deflate_file)(ZCompress *c, char *file_in, char *file_out);
    int (*inflate_file)(ZCompress *c, char *file_in, char *file_out);
};

#endif
