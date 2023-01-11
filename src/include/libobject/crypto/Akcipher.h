#ifndef __AKCIPHER_H__
#define __AKCIPHER_H__

#include <stdio.h>
#include <libobject/core/Obj.h>

typedef struct Akcipher_s Akcipher;

typedef enum {
    AKCIPHER_KEY_TYPE_UNKNOWN = 0,
    AKCIPHER_KEY_TYPE_PRIVATE,
    AKCIPHER_KEY_TYPE_PUBLIC,
    AKCIPHER_KEY_TYPE_MAX,
} akcipher_key_type_e;

struct Akcipher_s{
    Obj parent;

    int (*construct)(Akcipher *,char *);
    int (*deconstruct)(Akcipher *);

    /*virtual methods reimplement*/
    int (*generate_keys)(Akcipher *, int key_len);
    int (*get_public_key)(Akcipher *, void **key);
    int (*get_private_key)(Akcipher *, void **key);
    int (*compare_keys)(Akcipher *cipher, void *key1, void *key2);
    int (*encrypt)(Akcipher *, akcipher_key_type_e type, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
    int (*decrypt)(Akcipher *, akcipher_key_type_e type, const u8 *in, const u32 in_len, u8 *out, u32 *out_len);
};

#endif
