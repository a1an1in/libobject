#ifndef __DIDEST_SHA1_H__
#define __DIDEST_SHA1_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/encoding/Digest.h>

typedef struct Digest_Sha1_s Digest_Sha1;

struct Digest_Sha1_s{
    Digest parent;

    int (*construct)(Digest *,char *);
    int (*deconstruct)(Digest *);

    /*virtual methods reimplement*/
    int (*set)(Digest *digest, char *attrib, void *value);
    void *(*get)(Digest *, char *attrib);
    int (*init)(Digest *digest);
    int (*update)(Digest *digest, const void *data, unsigned int size);
    int (*final)(Digest *digest, uint8_t *result, unsigned int size);

    void *context;
};

#endif
