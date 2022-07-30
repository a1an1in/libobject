#ifndef __CipherMode_H__
#define __CipherMode_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/crypto/SkcipherAlgo.h>

typedef struct CipherMode_s SkcipherMode;

struct CipherMode_s{
    Obj parent;

    int (*construct)(SkcipherMode *,char *);
    int (*deconstruct)(SkcipherMode *);

    /*virtual methods reimplement*/
    int (*set)(SkcipherMode *, char *attrib, void *value);
    void *(*get)(SkcipherMode *, char *attrib);
    int (*create)(SkcipherMode *, char *sub_algo, SkcipherAlgo *algo);
};

#endif
