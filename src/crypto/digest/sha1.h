#ifndef __DIGEST_SHA1__
#define __DIGEST_SHA1__

#include <libobject/basic_types.h>

typedef struct {
    unsigned long long bytes;
    uint32_t a, b, c, d, e, f;
    uint8_t buffer[64];
} digest_sha1_t;

void digest_sha1_init(digest_sha1_t *ctx);
void digest_sha1_update(digest_sha1_t *ctx, const void *data, unsigned int size);
void digest_sha1_final(digest_sha1_t *ctx, uint8_t result[20]);

#endif
