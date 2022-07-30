#ifndef __SkcipherModeCbc_H__
#define __SkcipherModeCbc_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/crypto/SkcipherMode.h>

typedef struct SkcipherModeCbc_s SkcipherModeCbc;

struct SkcipherModeCbc_s{
    SkcipherMode parent;

    int (*construct)(SkcipherModeCbc *,char *);
    int (*deconstruct)(SkcipherModeCbc *);

    /*virtual methods reimplement*/
    int (*set)(SkcipherModeCbc *ak, char *attrib, void *value);
    void *(*get)(SkcipherModeCbc *, char *attrib);
    int (*create)(SkcipherModeCbc *, char *sub_algo, SkcipherAlgo *algo);
};

#endif
