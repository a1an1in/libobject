#ifndef __AKCIPHER_H__
#define __AKCIPHER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Akcipher_s Akcipher;

struct Akcipher_s{
    Obj parent;

    int (*construct)(Akcipher *,char *);
    int (*deconstruct)(Akcipher *);

    /*virtual methods reimplement*/
    int (*generate_keys)(Akcipher *, int key_len);
    int (*get_public_key)(Akcipher *);
    int (*get_private_key)(Akcipher *);
    int (*encrypt)(Akcipher *, void *key, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*decrypt)(Akcipher *, void *key, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
};

#endif
