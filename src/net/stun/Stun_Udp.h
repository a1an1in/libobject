
#ifndef __STUN_UDP_H__
#define __STUN_UDP_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/concurrent/net/api.h>
#include "Stun.h"

typedef struct Stun_Udp_s Stun_Udp;

struct Stun_Udp_s{
    Stun parent;

    int (*construct)(Stun_Udp *,char *);
    int (*deconstruct)(Stun_Udp *);

    /*virtual methods reimplement*/
    int (*set)(Stun_Udp *, char *attrib, void *value);
    void *(*get)(Stun_Udp *, char *attrib);
    char *(*to_json)(Stun_Udp *); 
    int (*connect)(Stun_Udp *stun, char *host, char *service);
    int (*send)(Stun_Udp *stun);
    int (*discovery)(Stun_Udp *stun);
    int (*set_read_post_callback)(Stun_Udp *stun, int (*func)(Response *, void *arg));

    Client *c;
};

#endif
