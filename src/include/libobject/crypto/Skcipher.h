#ifndef __SKCIPHER_H__
#define __SKCIPHER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/crypto/CipherMode.h>
#include <libobject/crypto/CipherAlgo.h>

typedef struct Skcipher_s Skcipher;
#define MAX_MODE_NAME_LEN 20
#define MAX_SUB_ALGO_NAME 40

struct Skcipher_s{
    Obj parent;

    int (*construct)(Skcipher *,char *);
    int (*deconstruct)(Skcipher *);

    /*virtual methods reimplement*/
    int (*set)(Skcipher *sk, char *attrib, void *value);
    void *(*get)(Skcipher *, char *attrib);
    int (*set_key)(Skcipher *sk, char *key, void *key_len);
    int (*set_algo)(Skcipher *sk, char *algo);
    int (*encrypt)(Skcipher *sk, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*decrypt)(Skcipher *sk, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*set_padding)(Skcipher *sk, SkcipherPaddingEnum padding);

    /* attribs */
    void *key;
    int key_len;
    CipherAlgo *algo;
    CipherMode *mode;
    String *str;
};

#endif
