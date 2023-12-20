#ifndef __DIDEST_HMAC_SHA1_V1_H__
#define __DIDEST_HMAC_SHA1_V1_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/encoding/Digest.h>
#include <openssl/hmac.h>

typedef struct Digest_HmacSha1_s Digest_HmacSha1;

struct Digest_HmacSha1_s{
    Digest parent;

    int (*construct)(Digest *,char *);
    int (*deconstruct)(Digest *);

    /*virtual methods reimplement*/
    int (*set)(Digest *digest, char *attrib, void *value);
    void *(*get)(Digest *, char *attrib);
    int (*init)(Digest *digest);
    int (*init_with_key)(Digest *digest, char *key, int len);
    int (*update)(Digest *digest, const void *data, unsigned int size);
    int (*final)(Digest *digest, uint8_t *result, unsigned int size);

    HMAC_CTX *context;
};

#endif
