#ifndef __Aes_H__
#define __Aes_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Aes_s Aes;

struct Aes_s{
    Obj parent;

    int (*construct)(Aes *,char *);
    int (*deconstruct)(Aes *);

    /*virtual methods reimplement*/
    int (*set)(Aes *, char *attrib, void *value);
    void *(*get)(Aes *, char *attrib);
    char *(*to_json)(Aes *); 
    int (*set_key)(Aes *sk, char *key, void *key_len);
    int (*encrypt)(Aes *sk, void *in, int in_len, void *out, int out_len);
    int (*decrypt)(Aes *sk, void *in, int in_len, void *out, int out_len);
};

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
