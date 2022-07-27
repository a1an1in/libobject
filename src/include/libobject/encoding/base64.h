#ifndef __BASE64_H__
#define __BASE64_H__

int base64_encode(uint8_t *src, int src_len, uint8_t *dst, int dst_len); 
int base64_decode(uint8_t *src, int src_len, uint8_t *dst, int dst_len);

#endif