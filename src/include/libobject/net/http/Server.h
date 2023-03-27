#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/net/http/http_status.h>
#include <libobject/net/http/Request.h>
#include <libobject/net/http/Response.h>
#include <libobject/net/http/handler.h>
#include <libobject/net/worker/api.h>

#define HTTP_MAX_FILE_LEN 1024

typedef struct http_server_s Http_Server;

struct http_server_s{
    Obj obj;

    int (*construct)(Http_Server *,char *init_str);
    int (*deconstruct)(Http_Server *);
    int (*set)(Http_Server *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*start)(Http_Server *);
    int (*process_request)(Http_Server *hs, Request *r);
    int (*register_handler)(Http_Server *hs, char *method, char *path, int (*handler)(Request *, Response *, void *), void *opaque);
    int (*response)(Http_Server *hs, Request *req, Response *res);
    int (*override_inner_handler)(Http_Server *server, char *key, int (*handler)(Request *, Response *, void *));

    /*attribs*/
    Server *s;
    Map *get_handlers;
    Map *post_handlers;
    Map *other_handlers;
    String *root;
    String *host;
    String *service;
};

#define MAX_FILE_LEN 1024

typedef enum form_data_type_e {
    E_FORM_DATA_TYPE_IMAGE = 0,
    E_FORM_DATA_TYPE_VIDEO,
    E_FORM_DATA_TYPE_UNKNOW,
} Form_Data_Type;

#endif

