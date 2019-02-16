#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/io/RingBuffer.h>
#include <libobject/core/string.h>
typedef enum response_status
{
    /* data */
    response_success,
    response_timeout,
    response_ok,
    response_server_error,
    response_connect_error,
    response_bad_request,
    response_redirect
}response_status_t;

typedef struct response_s Response;

struct response_s{
	Obj obj;

	int (*construct)(Response *,char *init_str);
	int (*deconstruct)(Response *);
	int (*set)(Response *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*set_buffer)(Response *response, RingBuffer *buffer);
    int (*read)(Response *response);
    int (*response_parse)(Response *response,void *buffer,int len);
    int current_size;
    int content_length;
    response_status_t response_status;
    RingBuffer *buffer;
    String *response_context;
};

#endif
