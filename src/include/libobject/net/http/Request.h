#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/map.h>

typedef struct request_s Request;

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
    int (*set_body)(Request *request, void *body);
    int (*write)(Request *request);

    /*attribs*/
    Map *header;
    void *method;
    void *uri;
    void *version;
    void *body;
};

#endif
