#ifndef __ADDR_CONVER_H__
#define __ADDR_CONVER_H__

#include <libobject/basic_types.h>

void addr_to_buffer(void *addr,uint8_t *buffer);
void *buffer_to_addr(uint8_t *buffer);
void printf_buffer(unsigned char *buf, int len);

#endif
