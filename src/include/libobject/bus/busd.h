#ifndef __BUSD_H__
#define __BUSD_H__

#include <libobject/basic_types.h>
#include <libobject/utils/data_structure/vector.h>
#include <libobject/utils/blob/blob.h>
#include <libobject/net/server/server.h>
#include <libobject/core/map.h>

enum {
	BUSD_ID,
	BUSD_OBJNAME,
	BUSD_METHORDS,
    BUSD_INVOKE_KEY,
	BUSD_INVOKE_METHORD,
    BUSD_INVOKE_ARGC,
    BUSD_INVOKE_ARGS,
    BUSD_STATE,
    BUSD_OPAQUE,
    BUSD_INVOKE_SRC_FD,
	__BUSD_MAX
};

typedef struct busd_task_s{
    int fd;
}busd_task_t;

typedef struct busd_s{
    allocator_t *allocator;
    Server *server;
    char *server_sk_type;
    char *server_host;
    char *server_srv;

    blob_t *blob;
	Map *obj_map;
    uint8_t key_size;
    uint8_t bucket_size;
}busd_t;

struct busd_object_method_arg_s{
	char *name;
	uint8_t type;
};
struct busd_object_method {
	char *name;
	llist_t *args;
};

typedef struct busd_object {
	char *name;
	uint32_t id;
	vector_t *methods;
    uint8_t fd;
    allocator_t *allocator;
}busd_object_t;

typedef int (*busd_cmd_callback)(busd_t *busd,  struct blob_attr_s **attr,int fd);
busd_t *busd_create(allocator_t *allocator,
                    char *server_host,
                    char *server_srv,
                    char *socket_type);
int busd_destroy(busd_t *busd);

#endif
