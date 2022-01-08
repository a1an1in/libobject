
#ifndef __STUN_H__
#define __STUN_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/net/client/Client.h>

typedef struct Stun_s Stun;

struct Stun_s{
    Obj parent;

    int (*construct)(Stun *,char *);
    int (*deconstruct)(Stun *);

    /*virtual methods reimplement*/
    int (*set)(Stun *module, char *attrib, void *value);
    void *(*get)(Stun *, char *attrib);
    char *(*to_json)(Stun *); 
};

#endif
