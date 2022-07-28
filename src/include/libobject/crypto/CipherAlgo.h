#ifndef __CipherAlgo_H__
#define __CipherAlgo_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct CipherAlgo_s CipherAlgo;
typedef int (*CRIPTO_FUNC)(CipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 out_len);

typedef enum {
    SKCIPHER_PADDING_NO = 0,
    SKCIPHER_PADDING_ZERO,
    SKCIPHER_PADDING_PKCS7
} SkcipherPaddingEnum;

struct CipherAlgo_s{
    Obj parent;

    int (*construct)(CipherAlgo *,char *);
    int (*deconstruct)(CipherAlgo *);

    /*virtual methods reimplement*/
    int (*set)(CipherAlgo *, char *attrib, void *value);
    void *(*get)(CipherAlgo *, char *attrib);
    int (*set_key)(CipherAlgo *, char *key, void *key_len);
    int (*encrypt)(CipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 out_len);
    int (*decrypt)(CipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 out_len);

    /* attribs */
    void *key;
    int key_len;
    void *sub_algo;
    SkcipherPaddingEnum padding;
};

#endif
