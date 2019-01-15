#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/io/Buffer.h>

typedef struct response_s Response;

struct response_s{
	Obj obj;

	int (*construct)(Response *,char *init_str);
	int (*deconstruct)(Response *);
	int (*set)(Response *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*set_buffer)(Response *response, Buffer *buffer);
    int (*read)(Response *response);

    Buffer *buffer;

};

#endif
