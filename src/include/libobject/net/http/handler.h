#ifndef __HTTP_HANDLE_H__
#define __HTTP_HANDLE_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/net/http/Request.h>
#include <libobject/net/http/Response.h>

typedef struct handler_s{
    char *method;
    char *path;
    int (*callback)(Request *, Response *, void *);
    void *opaque;
} handler_t;

int __handler_bad_request(Request *req, Response *res, void *opaque);
int __handler_not_found(Request *req, Response *res, void *opaque);
int __handler_get_directory_list(Request *req, Response *res, void *opaque);
#endif
