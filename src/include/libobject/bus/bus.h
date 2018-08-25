#ifndef __BUS_H__
#define __BUS_H__

#include <libobject/net/client/client.h>
#include <libobject/net/net_task.h>
#include <libobject/core/utils/blob/blob.h>
#include <libobject/core/map.h>

enum {
	BUS_ID,
	BUS_OBJNAME,
	BUS_OBJINFOS,
	BUS_STATE,
	BUS_OPAQUE,
	BUS_INVOKE_SRC_FD,
	BUS_INVOKE_DST_FD,
	BUS_INVOKE_METHOD,
	BUS_INVOKE_ARGC,
	BUS_INVOKE_ARGS,
	__BUS_MAX
};

struct bus_s;
typedef struct bus_s bus_t;

typedef int (*bus_handler_t)(struct bus_s *bus,
                             int argc,
		      		         struct blob_attr_s **args,
                             void *out_data,
                             int *out_data_len);
typedef int (*bus_cmd_callback)(bus_t *bus,  blob_attr_t **attr);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define BUS_METHOD(_name, _handler, _policy)\
{\
	.name = _name,\
	.handler = _handler,\
	.policy = (struct blob_policy_s *)_policy,\
	.n_policy = ARRAY_SIZE(_policy),\
}

enum bus_method_arg_type_e {       
    ARG_TYPE_UNSPEC,  
    ARG_TYPE_ARRAY,
    ARG_TYPE_TABLE,
    ARG_TYPE_STRING,
    ARG_TYPE_BUFFER,
    ARG_TYPE_INT64,
    ARG_TYPE_INT32,
    ARG_TYPE_INT16,
    ARG_TYPE_INT8,
    ARG_TYPE_LAST,
}; 
typedef struct bus_method_args_s{
    uint8_t type;
    char *name;
    char *value;
}bus_method_args_t;

typedef struct bus_reqhdr {
	uint8_t version;
	uint8_t type;
	uint32_t s_id;
	uint32_t d_id;
}__attribute__((packed)) bus_reqhdr_t;

enum bus_communication_type {
	/* initial server message */
	BUS_REQ_HELLO,
	/* generic command response */
	BUS_REQ_STATUS,
	/* data message response */
	BUS_REQ_DATA,
	/* ping request */
	BUS_REQ_PING,
	/* look up one or more objects */
	BUS_REQ_LOOKUP,
	BUSD_REPLY_LOOKUP,
	/* invoke a method on a single object */
	BUS_REQ_INVOKE,
	BUSD_REPLY_INVOKE,
	BUSD_FORWARD_INVOKE,
	BUS_REPLY_FORWARD_INVOKE,
	BUS_REQ_ADD_OBJECT,
	BUSD_REPLY_ADD_OBJECT,
	BUS_REQ_REMOVE_OBJECT,
	/*
	 * subscribe/unsubscribe to object notifications
	 * The unsubscribe message is sent from busd when
	 * the object disappears
	 */
	BUS_REQ_SUBSCRIBE,
	BUS_REQ_UNSUBSCRIBE,
	/*
	 * send a notification to all subscribers of an object.
	 * when sent from the server, it indicates a subscription
	 * status change
	 */
	BUS_REQ_NOTIFY,
	/* must be last */
	__BUS_REQ_LAST,
};

struct bus_method {
	char *name;
	bus_handler_t handler;
	struct blob_policy_s *policy;
	int n_policy;
};

typedef struct bus_object {
	char *name;
	uint32_t id;
	char *path;
	struct bus_method *methods;
	int n_methods;
}bus_object_t;

typedef struct bus_req_s{
    char *method;
    short state;
#define BUS_REQ_MAX_OPAQUE_LEN 1024
    uint8_t *opaque;
#undef BUS_REQ_MAX_OPAQUE_LEN
    uint16_t opaque_len;
    uint16_t opaque_buffer_len;
    int read, write;
}bus_req_t;

struct bus_s{
    allocator_t *allocator;
    Client *client;
    char *client_sk_type;
    char *server_host;
    char *server_srv;
    blob_t *blob;

	Map *obj_map;
    uint8_t key_size;
    uint8_t bucket_size;

	Map *req_map;
    uint8_t req_key_size;
    uint8_t req_bucket_size;
};


bus_t * bus_alloc(allocator_t *allocator);
int bus_init(bus_t *bus,
             char *server_host,
             char *server_srv,
             int (*process_client_task_cb)(void *task));
int bus_send(bus_t *bus,
			 void *buf,
			 size_t buf_len);

int bus_add_object(bus_t *bus,struct bus_object *obj);

int bus_invoke_sync(bus_t *bus,char *key, char *method,int argc, bus_method_args_t *args,char *out_buf,char *out_len);

bus_t * bus_create(allocator_t *allocator,
                   char *server_host,
                   char *server_srv,
                   char *socket_type);
int bus_destroy(bus_t *bus);
#endif
