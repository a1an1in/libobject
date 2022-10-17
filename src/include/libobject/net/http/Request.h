#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/core/Object_Chain.h>
#include <libobject/core/io/Ring_Buffer.h>
#include <libobject/net/http/http_status.h>
#include <libobject/core/io/Socket.h>
#include <libobject/core/io/File.h>

typedef struct request_s Request;

enum request_type_e {
    REQUEST_TYPE_NOT_DEFINED = 0,
    REQUEST_TYPE_STATIC,
    REQUEST_TYPE_CGI,
    REQUEST_TYPE_DYNAMIC,
};

struct request_s{
    Obj obj;

    int (*construct)(Request *,char *init_str);
    int (*deconstruct)(Request *);
    int (*set)(Request *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods reimplement*/
    int (*set_method)(Request *request, void *method);
    int (*set_uri)(Request *request, void *url);
    int (*set_http_version)(Request *request, void *version);
    int (*set_header)(Request *request, void *key, void *value);
    int (*set_body)(Request *request, void *body, int len);
    int (*write)(Request *request);
    int (*read)(Request *request);
    char *(*get_form_value)(Request *request, char *form_name);
    int (*get_request_type)(Request *request);
    int (*get_cookie)(Request *request, char *key, char *value, int value_len);

    /*attribs*/
    void *method;
    void *uri;
    void *query;
    void *version;
    Buffer *body;
    int content_len;
    Map *forms;
    //Map *post_forms;
    Map *headers;
    Ring_Buffer *buffer;
    Object_Chain *chain;
    enum http_status_e status;
    int connect_fd;

    /* extern attribs */
    String *cookies;
    File *file; //this is alloc by chain, not need req to release;
    void *response;
    Socket *socket;
    void *server;
};

#endif
