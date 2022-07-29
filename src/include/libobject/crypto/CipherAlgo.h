#ifndef __CipherAlgo_H__
#define __CipherAlgo_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct CipherAlgo_s CipherAlgo;
typedef int (*CRIPTO_FUNC)(CipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 out_len);
#define MAX_BLOCK_SIZE 128

typedef enum {
    SKCIPHER_PADDING_NO = 0,
    SKCIPHER_PADDING_ZERO,
    SKCIPHER_PADDING_PKCS7,
    SKCIPHER_PADDING_MAX
} SkcipherPaddingEnum;

struct CipherAlgo_s{
    Obj parent;

    int (*construct)(CipherAlgo *,char *);
    int (*deconstruct)(CipherAlgo *);

    /*virtual methods reimplement*/
    int (*set)(CipherAlgo *, char *attrib, void *value);
    void *(*get)(CipherAlgo *, char *attrib);
    int (*set_key)(CipherAlgo *, char *key, void *key_len);
    int (*encrypt)(CipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*decrypt)(CipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*pad)(CipherAlgo *, const u8 *in, const u32 in_len);
    int (*unpad)(CipherAlgo *, u8 *out, u32 *out_len);
    int (*get_block_size)(CipherAlgo *, u32 *size);

    /* attribs */
    void *key;
    int key_len;
    void *sub_algo;
    SkcipherPaddingEnum padding;
    uint8_t last_block[MAX_BLOCK_SIZE];
};

#endif
