#ifndef __DIGEST_CRC__
#define __DIGEST_CRC__

#include <libobject/basic_types.h>

uint16_t crc16(uint16_t crc, const char *buf, int len);
static unsigned int crc32(unsigned int crc, const unsigned char *buf, unsigned int size);
uint64_t crc64(uint64_t crc, const unsigned char *buf, uint64_t l);
int file_compute_crc32(char *file_name, uint32_t *crc);

#endif
