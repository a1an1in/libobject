#ifndef __Aes_H__
#define __Aes_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/crypto/Skcipher.h>>

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
    Skcipher parent;

    int (*construct)(Aes *,char *);
    int (*deconstruct)(Aes *);

    /*virtual methods reimplement*/
    int (*set)(Aes *, char *attrib, void *value);
    void *(*get)(Aes *, char *attrib);
    char *(*to_json)(Aes *); 
    int (*set_key)(Aes *sk, char *key, void *key_len);
    int (*encrypt)(Aes *ctx, const u8 *in, u8 *out);
    int (*decrypt)(Aes *ctx, const u8 *in, u8 *out);

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

// #define f_rn(bo, bi, n, k)	do {				                               \
// 	bo[n] = crypto_ft_tab[0][byte(bi[n], 0)] ^			                       \
// 		crypto_ft_tab[1][byte(bi[(n + 1) & 3], 1)] ^		                   \
// 		crypto_ft_tab[2][byte(bi[(n + 2) & 3], 2)] ^		                   \
// 		crypto_ft_tab[3][byte(bi[(n + 3) & 3], 3)] ^ *(k + n);             	   \
// } while (0)

// #define f_nround(bo, bi, k)	do {                                               \
// 	f_rn(bo, bi, 0, k);	                                                       \
// 	f_rn(bo, bi, 1, k);		                                                   \
// 	f_rn(bo, bi, 2, k); 	                                                   \
// 	f_rn(bo, bi, 3, k);	                                                       \
// 	k += 4;	                                                       	           \
// } while (0)

// #define f_rl(bo, bi, n, k)	do {	                                           \
// 	bo[n] = crypto_fl_tab[0][byte(bi[n], 0)] ^	                               \
// 		crypto_fl_tab[1][byte(bi[(n + 1) & 3], 1)] ^	                       \
// 		crypto_fl_tab[2][byte(bi[(n + 2) & 3], 2)] ^	                       \
// 		crypto_fl_tab[3][byte(bi[(n + 3) & 3], 3)] ^ *(k + n);	               \
// } while (0)

// #define f_lround(bo, bi, k)	do {	                                           \
// 	f_rl(bo, bi, 0, k);	                                                       \
// 	f_rl(bo, bi, 1, k);	                                                       \
// 	f_rl(bo, bi, 2, k);	                                                       \
// 	f_rl(bo, bi, 3, k);	                                                       \
// } while (0)

// #define i_rn(bo, bi, n, k)	do {                                               \
// 	bo[n] = crypto_it_tab[0][byte(bi[n], 0)] ^                                 \
// 		crypto_it_tab[1][byte(bi[(n + 3) & 3], 1)] ^                           \
// 		crypto_it_tab[2][byte(bi[(n + 2) & 3], 2)] ^                           \
// 		crypto_it_tab[3][byte(bi[(n + 1) & 3], 3)] ^ *(k + n);                 \
// } while (0)

// #define i_nround(bo, bi, k)	do {                                               \
// 	i_rn(bo, bi, 0, k);	                                                       \
// 	i_rn(bo, bi, 1, k);	                                                       \
// 	i_rn(bo, bi, 2, k);	                                                       \
// 	i_rn(bo, bi, 3, k);	                                                       \
// 	k += 4;	                                                       	           \
// } while (0)

// #define i_rl(bo, bi, n, k)	do {	                                           \
// 	bo[n] = crypto_il_tab[0][byte(bi[n], 0)] ^	                               \
// 	crypto_il_tab[1][byte(bi[(n + 3) & 3], 1)] ^	                           \
// 	crypto_il_tab[2][byte(bi[(n + 2) & 3], 2)] ^	                           \
// 	crypto_il_tab[3][byte(bi[(n + 1) & 3], 3)] ^ *(k + n);	                   \
// } while (0)

// #define i_lround(bo, bi, k)	do {	                                           \
// 	i_rl(bo, bi, 0, k);	                                                       \
// 	i_rl(bo, bi, 1, k);	                                                       \
// 	i_rl(bo, bi, 2, k);	                                                       \
// 	i_rl(bo, bi, 3, k);	                                                       \
// } while (0)

#endif
