#ifndef __DIDEST_H__
#define __DIDEST_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Digest_s Digest;

struct Digest_s{
    Obj parent;

    int (*construct)(Digest *,char *);
    int (*deconstruct)(Digest *);

    /*virtual methods reimplement*/
    int (*set)(Digest *digest, char *attrib, void *value);
    void *(*get)(Digest *, char *attrib);
    char *(*to_json)(Digest *); 
};

#endif
