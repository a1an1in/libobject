#ifndef __DIGEST_MD5__
#define __DIGEST_MD5__

#include <libobject/basic_types.h>

typedef struct {
    unsigned long long bytes;
    uint32_t a, b, c, d;
    uint8_t buffer[64];
} digest_md5_t;

void digest_md5_init(digest_md5_t *ctx);
void digest_md5_update(digest_md5_t *ctx, const void *data, unsigned int size);
void digest_md5_final(digest_md5_t *ctx, uint8_t result[16]);

#endif 
