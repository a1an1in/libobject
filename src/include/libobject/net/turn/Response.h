#ifndef __TURN_RESPONSE_H__
#define __TURN_RESPONSE_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/core/io/Ring_Buffer.h>
#include "protocol.h"

typedef struct Response_s Response;


struct Response_s{
    Obj parent;

    int (*construct)(Response *,char *);
    int (*deconstruct)(Response *);

    /*virtual methods reimplement*/
    int (*set)(Response *, char *attrib, void *value);
    void *(*get)(Response *, char *attrib);
    char *(*to_json)(Response *); 
    int (*read)(Response *response);

    turn_header_t *header;
    int header_max_len;
    int len;
    turn_attribs_t attribs;
    Ring_Buffer *buffer;
    int (*read_post_callback)(Response *, void *arg);
};

#endif
