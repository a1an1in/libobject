#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/core/Object_Chain.h>
#include <libobject/core/io/Ring_Buffer.h>
#include <libobject/core/io/Buffer.h>
#include <libobject/net/http/http_status.h>
#include <libobject/core/io/Socket.h>

typedef struct response_s Response;

struct response_s{
    Obj obj;

    int (*construct)(Response *,char *init_str);
    int (*deconstruct)(Response *);
    int (*set)(Response *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*read)(Response *response);
    int (*set_status_code)(Response *response, int status);
    int (*get_status_code)(Response *response);
    int (*set_body)(Response *response, void *body, int len);
    int (*set_fmt_body)(Response *response, int max_len, char *fmt, ...);
    char *(*get_body)(Response *response);
    int (*set_header)(Response *response, void *key, void *value);
    int (*write)(Response *response);

    Map *headers;
    Ring_Buffer *buffer;
    Object_Chain *chain;
    enum http_status_e status;
    int status_code;
    Buffer *body;
    FILE *file;
    int len;
    int content_len;
    uint8_t end_flag;
    Socket *socket;
    uint8_t writable_flag;
};

#endif
