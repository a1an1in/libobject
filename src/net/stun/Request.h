#ifndef __STUN_REQUEST_H__
#define __STUN_REQUEST_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include "stun_header.h"


typedef struct Request_s Request;

struct Request_s{
    Obj parent;

    int (*construct)(Request *,char *);
    int (*deconstruct)(Request *);

    /*virtual methods reimplement*/
    int (*set)(Request *module, char *attrib, void *value);
    void *(*get)(Request *, char *attrib);
    char *(*to_json)(Request *); 
    int (*write_head)(Request *request, int type, int len, uint32_t cookie);
    int (*write_attrib)(Request *request, int type, int len, char *value);

    stun_header_t *header;
    int len;
    Map *attribs;
};

#endif
