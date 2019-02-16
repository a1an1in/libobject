#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/rbtree_map.h>
#include <libobject/io/RingBuffer.h>
#include <libobject/core/string.h>
#include <libobject/concurrent/worker.h>

#define HTTP_REQUEST_MAX_BUFFER  1024
#define HTTP_REQUEST_MIN_BUFFER  100
#define HTTP_REQUEST_MID_BUFFER  500




typedef enum HTTP_OPTIONS{
    HTTP_OPT_PROXY = 1,
    HTTP_OPT_EFFECTIVE_URL,
    HTTP_OPT_USERAGENT,
    HTTP_OPT_USERPWD,
    HTTP_OPT_JSON_FORMAT,
    HTTP_OPT_TIMEOUT,
    HTTP_OPT_ENCODING,
    HTTP_OPT_CONNECTTIMEOUT, 
    HTTP_OPT_REFER,
    HTTP_OPT_ACCEPT,
    HTTP_OPT_SSLKEY,
    HTTP_OPT_METHOD_POST,
    HTTP_OPT_METHOD_GET,
    HTTP_OPT_CONTENT_LEN,
    HTTP_OPT_POSTFIELDS,
    HTTP_OPT_ACCEPT_LAN,
    HTTP_OPT_CONNTENT_TYPE,
    HTTP_OPT_HOST,
    HTTP_OPT_ACCEPT_CHARSET,
    HTTP_OPT_VERSION,
    HTTP_OPT_METHOD,
    HTTP_OPT_PORT,
    HTTP_OPT_CONNTENT_LEN,
    HTTP_OPT_CALLBACK,
    HTTP_OPT_CALLBACKDATA
}http_opt_t;

typedef enum REQUEST_STATE{
    REQUEST_STATE_OK,
    REQUEST_STATE_ERROR,
    REQUEST_STATE_TIMEOUT,
    REQUEST_STATE_CONNECTTIMEOUT,
    REQUEST_STATE_PROXY_ERROR
}request_state_t;

typedef int (*request_cb_t)(void *);

typedef struct request_s Request;

struct request_s{
	Obj obj;

	int (*construct)(Request *,char *init_str);
	int (*deconstruct)(Request *);
	int (*set)(Request *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/

    int (*set_content_len)(Request *request, int content_len);
    int (*write)(Request *request);

    int (*set_opt)(Request *,http_opt_t,void *);
    int (*option_valid)(Request *,http_opt_t);
    void (*option_reset)(Request *);
   
    int (*request_cb)(void *arg);
    void *request_cb_arg;

    /*attribs*/
    Worker *timer;
    struct timeval ev_tv;
    String * method;
    String * url;
    String * version;
    String * arguments;
    String *port;
    String *server_ip;
    int content_len;
    request_state_t request_result;
    String * request_header_context;
    RBTree_Map *map;
};

#endif
