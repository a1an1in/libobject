#ifndef __CipherAlgo_H__
#define __CipherAlgo_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct CipherAlgo_s CipherAlgo;

struct CipherAlgo_s{
    Obj parent;

    int (*construct)(CipherAlgo *,char *);
    int (*deconstruct)(CipherAlgo *);

    /*virtual methods reimplement*/
    int (*set)(CipherAlgo *, char *attrib, void *value);
    void *(*get)(CipherAlgo *, char *attrib);
    int (*set_key)(CipherAlgo *, char *key, void *key_len);
    int (*encrypt)(CipherAlgo *, const u8 *in, u8 *out);
    int (*decrypt)(CipherAlgo *, const u8 *in, u8 *out);

    /* attribs */
    void *key;
    int key_len;
};

#endif
