#ifndef __Aes_H__
#define __Aes_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/crypto/SkcipherAlgo.h>

typedef struct Aes_s Aes;
#define AES_MIN_KEY_SIZE	16
#define AES_MAX_KEY_SIZE	32
#define AES_KEYSIZE_128		16
#define AES_KEYSIZE_192		24
#define AES_KEYSIZE_256		32
#define AES_BLOCK_SIZE		16
#define AES_MAX_KEYLENGTH	(15 * 16)
#define AES_MAX_KEYLENGTH_U32	(AES_MAX_KEYLENGTH / sizeof(u32))

struct Aes_s{
    SkcipherAlgo parent;

    int (*construct)(Aes *,char *);
    int (*deconstruct)(Aes *);

    /*virtual methods reimplement*/
    int (*set)(Aes *, char *attrib, void *value);
    void *(*get)(Aes *, char *attrib);
    char *(*to_json)(Aes *); 
    int (*set_key)(Aes *sk, char *key, void *key_len);
    int (*encrypt)(Aes *ctx, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*decrypt)(Aes *ctx, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*get_block_size)(Aes *ctx, u32 *size);

    u32 key_enc[AES_MAX_KEYLENGTH_U32];
	u32 key_dec[AES_MAX_KEYLENGTH_U32];
	u32 key_length;
};

static inline void put_unaligned_u32(uint32_t val, void *p)
{
	memcpy(p, &val, sizeof(uint32_t));
}

static inline u32 get_unaligned_u32(const void *p)
{
    uint32_t val;
    memcpy(&val, p, sizeof(uint32_t));
	return val;
}

#endif
