#ifndef __SKCIPHER_H__
#define __SKCIPHER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Skcipher_s Skcipher;

struct Skcipher_s{
    Obj parent;

    int (*construct)(Skcipher *,char *);
    int (*deconstruct)(Skcipher *);

    /*virtual methods reimplement*/
    int (*set)(Skcipher *sk, char *attrib, void *value);
    void *(*get)(Skcipher *, char *attrib);
};

#endif
