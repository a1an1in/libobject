#ifndef __SkcipherModeEcb_H__
#define __SkcipherModeEcb_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/crypto/SkcipherMode.h>

typedef struct SkcipherModeEcb_s SkcipherModeEcb;

struct SkcipherModeEcb_s{
    SkcipherMode parent;

    int (*construct)(SkcipherModeEcb *,char *);
    int (*deconstruct)(SkcipherModeEcb *);

    /*virtual methods reimplement*/
    int (*set)(SkcipherModeEcb *ak, char *attrib, void *value);
    void *(*get)(SkcipherModeEcb *, char *attrib);
    int (*create)(SkcipherModeEcb *, char *sub_algo, SkcipherAlgo *algo);
};

#endif
