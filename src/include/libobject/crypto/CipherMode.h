#ifndef __CipherMode_H__
#define __CipherMode_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct CipherMode_s CipherMode;

struct CipherMode_s{
    Obj parent;

    int (*construct)(CipherMode *,char *);
    int (*deconstruct)(CipherMode *);

    /*virtual methods reimplement*/
    int (*set)(CipherMode *ak, char *attrib, void *value);
    void *(*get)(CipherMode *, char *attrib);
    int (*create)(CipherMode *, char *algo_name);
};

#endif
