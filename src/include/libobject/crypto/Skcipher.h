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
    int (*set_key)(Skcipher *sk, char *key, void *key_len);
    int (*encrypt)(Skcipher *sk, void *in, int in_len, void *out, int out_len);
    int (*decrypt)(Skcipher *sk, void *in, int in_len, void *out, int out_len);

    /* attribs */
    void *key;
    int key_len;
};

#endif
