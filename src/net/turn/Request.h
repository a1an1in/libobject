#ifndef __TURN_REQUEST_H__
#define __TURN_REQUEST_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/io/Buffer.h>
#include "protocol.h"


typedef struct Request_s Request;

struct Request_s{
    Obj parent;

    int (*construct)(Request *,char *);
    int (*deconstruct)(Request *);

    /*virtual methods reimplement*/
    int (*set)(Request *module, char *attrib, void *value);
    void *(*get)(Request *, char *attrib);
    char *(*to_json)(Request *); 
    int (*set_head)(Request *request, int type, int len, uint32_t cookie);
    int (*set_attrib)(Request *request, int type, int len, char *value);

    turn_header_t *header;
    int header_max_len;
    int len;
    Map *attribs;
    Buffer *buffer;
};

#endif
