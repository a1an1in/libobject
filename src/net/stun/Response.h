#ifndef __STUN_RESPONSE_H__
#define __STUN_RESPONSE_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include "stun_header.h"

typedef struct Response_s Response;


struct Response_s{
    Obj parent;

    int (*construct)(Response *,char *);
    int (*deconstruct)(Response *);

    /*virtual methods reimplement*/
    int (*set)(Response *, char *attrib, void *value);
    void *(*get)(Response *, char *attrib);
    char *(*to_json)(Response *); 
    int (*write_head)(Response *request, int type, int len, uint32_t cookie);
    int (*write_attrib)(Response *request, int type, int len, char *value);

    stun_header_t *header;
    int len;
    Map *attribs;
};

#endif
