#ifndef __SkcipherAlgo_H__
#define __SkcipherAlgo_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct SkcipherAlgo_s SkcipherAlgo;
typedef int (*CRIPTO_FUNC)(SkcipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 out_len);
#define MAX_BLOCK_SIZE 128

typedef enum {
    SKCIPHER_PADDING_NO = 0,
    SKCIPHER_PADDING_ZERO,
    SKCIPHER_PADDING_PKCS7,
    SKCIPHER_PADDING_MAX
} SkcipherPaddingEnum;

struct SkcipherAlgo_s{
    Obj parent;

    int (*construct)(SkcipherAlgo *,char *);
    int (*deconstruct)(SkcipherAlgo *);

    /*virtual methods reimplement*/
    int (*set)(SkcipherAlgo *, char *attrib, void *value);
    void *(*get)(SkcipherAlgo *, char *attrib);
    int (*set_key)(SkcipherAlgo *, char *key, void *key_len);
    int (*encrypt)(SkcipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*decrypt)(SkcipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*pad)(SkcipherAlgo *, const u8 *in, const u32 in_len);
    int (*unpad)(SkcipherAlgo *, u8 *out, u32 *out_len);
    int (*get_block_size)(SkcipherAlgo *, u32 *size);
    int (*set_iv)(SkcipherAlgo *, void *iv);

    /* attribs */
    void *key;
    int key_len;
    void *sub_algo;
    SkcipherPaddingEnum padding;
    uint8_t last_block[MAX_BLOCK_SIZE];
    uint8_t iv[MAX_BLOCK_SIZE];
};

#endif
