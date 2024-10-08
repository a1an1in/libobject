#ifndef __BIG_NUMBER_H__
#define __BIG_NUMBER_H__

#include <stdio.h>
#include <libobject/basic_types.h>

#define BN_MUL_COMPUTE_LOW_AND_HIGTH(low, high, a, b) ({ \
    uint64_t ret = (uint64_t)(a) * (b);                  \
    (high) = ret >> 32; (low) = ret;                     \
})

#define BN_MUL_U32(r, a, w, c) {                         \
    uint32_t high, low, ret, tmp = (a);                  \
    ret =  (r);                                          \
    BN_MUL_COMPUTE_LOW_AND_HIGTH(low, high, w, tmp);     \
    ret += (c);                                          \
    (c) =  (ret < (c)) ? 1 : 0;                          \
    (c) += high;                                         \
    ret += low;                                          \
    (c) += (ret < low) ? 1 : 0;                          \
    (r) =  ret;                                          \
}

int bn_add(uint8_t *dest, int dest_len, int *dest_size, uint8_t *add, int add_size);
int bn_sub(uint8_t *dest, int dest_len, int *dest_size, int *neg_flag, uint8_t *sub, int sub_size);
int bn_cmp(uint8_t *n1, int n1_len, uint8_t *n2, int n2_len, int *out);
int bn_mul(uint8_t *dest, int dest_len, int *dest_size, uint8_t *a, int a_len, uint8_t *b, int b_len);
int bn_div(uint8_t *quotient, int quotient_len, int *quotient_size, 
           uint8_t *remainder, int remainder_len, int *remainder_size,
           uint8_t *a, int a_size, uint8_t *b, int b_size);
int bn_rand(uint8_t *dest, int dest_len, int *dest_size, int bits, int top, int bottom);
int bn_prime(uint8_t *dest, int dest_len, int *dest_size, int bits);
int bn_to_hex_str(uint8_t *src, int src_len, char *dest, int *dest_len);

#endif