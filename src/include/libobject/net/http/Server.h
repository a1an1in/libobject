#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/server/server.h>

typedef struct http_server_s Http_Server;

struct http_server_s{
	Obj obj;

	int (*construct)(Http_Server *,char *init_str);
	int (*deconstruct)(Http_Server *);
	int (*set)(Http_Server *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*serve)(Http_Server *);

    /*attribs*/
    Server *s;
};

#endif
