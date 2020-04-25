#ifndef __AKCIPHER_H__
#define __AKCIPHER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Akcipher_s Akcipher;

struct Akcipher_s{
    Obj parent;

    int (*construct)(Akcipher *,char *);
    int (*deconstruct)(Akcipher *);

    /*virtual methods reimplement*/
    int (*set)(Akcipher *ak, char *attrib, void *value);
    void *(*get)(Akcipher *, char *attrib);
    char *(*to_json)(Akcipher *); 
};

#endif
