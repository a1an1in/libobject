#ifndef __AkcipherDsa_H__
#define __AkcipherDsa_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/crypto/Akcipher.h>

typedef struct AkcipherDsa_s AkcipherDsa;

struct AkcipherDsa_s{
    Akcipher parent;

    int (*construct)(AkcipherDsa *,char *);
    int (*deconstruct)(AkcipherDsa *);

    /*virtual methods reimplement*/
    int (*generate_keys)(AkcipherDsa *, int key_len);
    int (*get_public_key)(AkcipherDsa *);
    int (*get_private_key)(AkcipherDsa *);
    int (*encrypt)(AkcipherDsa *, void *key, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*decrypt)(AkcipherDsa *, void *key, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);

    /* attribs */
    void *keys;
};

#endif
