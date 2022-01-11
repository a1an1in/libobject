#ifndef __STUN_H__
#define __STUN_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include "stun_header.h"
#include "Request.h"
#include "Response.h"

#define STUN_ATR_TYPE_MAPPED_ADDR   		0x0001
#define STUN_ATR_TYPE_RESPONSE_ADDRESS	    0x0002
#define STUN_ATR_TYPE_CHANGE_REQUEST	    0x0003
#define STUN_ATR_TYPE_SOURCE_ADDRESS	    0x0004
#define STUN_ATR_TYPE_CHANGED_ADDRESS	    0x0005
#define STUN_ATR_TYPE_USERNAME			    0x0006
#define STUN_ATR_TYPE_PASSWORD			    0x0007
#define STUN_ATR_TYPE_INTEGRITY		        0x0008
#define STUN_ATR_TYPE_ERROR_CODE			0x0009
#define STUN_ATR_TYPE_UNKNOWN_ATTRIBUTES	0x000a
#define STUN_ATR_TYPE_REFLECTED_FROM		0x000b
#define STUN_ATR_TYPE_XOR-MAPPED-ADDRESS	0x0020

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

#endif
