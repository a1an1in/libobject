#ifndef __BIG_NUMBER_H__
#define __BIG_NUMBER_H__

#include <stdio.h>
#include <libobject/basic_types.h>


int bn_add(uint8_t *dest, int dest_len, int *dest_size, uint8_t *add, int add_size);
#endif