#ifndef __AkcipherAlgo_H__
#define __AkcipherAlgo_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct AkcipherAlgo_s AkcipherAlgo;
typedef int (*CRIPTO_FUNC)(AkcipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 out_len);
#define MAX_BLOCK_SIZE 128

typedef enum {
    AKCIPHER_PADDING_NO = 0,
    AKCIPHER_PADDING_ZERO,
    AKCIPHER_PADDING_PKCS7,
    AKCIPHER_PADDING_MAX
} SkcipherPaddingEnum;

struct AkcipherAlgo_s{
    Obj parent;

    int (*construct)(AkcipherAlgo *,char *);
    int (*deconstruct)(AkcipherAlgo *);

    /*virtual methods reimplement*/
    int (*set)(AkcipherAlgo *, char *attrib, void *value);
    void *(*get)(AkcipherAlgo *, char *attrib);
    int (*set_key)(AkcipherAlgo *, char *key, void *key_len);
    int (*encrypt)(AkcipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*decrypt)(AkcipherAlgo *, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*pad)(AkcipherAlgo *, const u8 *in, const u32 in_len);
    int (*unpad)(AkcipherAlgo *, u8 *out, u32 *out_len);
    int (*get_block_size)(AkcipherAlgo *, u32 *size);


    /* attribs */
    void *key;
    int key_len;
    void *sub_algo;
    SkcipherPaddingEnum padding;
    uint8_t last_block[MAX_BLOCK_SIZE];
    uint8_t iv[MAX_BLOCK_SIZE];
};

#endif
