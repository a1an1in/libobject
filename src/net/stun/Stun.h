#ifndef __STUN_H__
#define __STUN_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include "stun_header.h"
#include "Request.h"
#include "Response.h"

typedef struct Stun_s Stun;

struct Stun_s{
    Obj parent;

    int (*construct)(Stun *,char *);
    int (*deconstruct)(Stun *);

    /*virtual methods reimplement*/
    int (*set)(Stun *module, char *attrib, void *value);
    void *(*get)(Stun *, char *attrib);
    char *(*to_json)(Stun *); 
    int (*connect)(Stun *stun, char *host, char *service);
    int (*send)(Stun *stun);
    int (*discovery)(Stun *stun);
    int (*set_read_post_callback)(Stun *stun, int (*func)(Response *, void *arg));

    Request *req;
    Response *response;
};

typedef struct attrib_parse_policy_s {
    int (*policy)(stun_attrib_t *, stun_attrib_t *);
} attrib_parse_policy_t;

attrib_parse_policy_t g_parse_attr_policies[ENTRY_TYPE_MAX_TYPE];

#endif
