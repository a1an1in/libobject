#ifndef __BIG_NUMBER_H__
#define __BIG_NUMBER_H__

#include <stdio.h>
#include <libobject/basic_types.h>

int bn_add(uint8_t *dest, int dest_len, int *dest_size, uint8_t *add, int add_size);
int bn_sub(uint8_t *dest, int dest_len, int *dest_size, int *neg_flag, uint8_t *sub, int sub_size);
int bn_cmp(uint8_t *n1, int n1_len, uint8_t *n2, int n2_len, int *out);
int bn_mul(uint8_t *dest, int dest_len, uint8_t *a, int a_len, int *b, int b_len);

#endif