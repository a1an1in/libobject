#ifndef __BUSD_H__
#define __BUSD_H__

#include <libobject/basic_types.h>
#include <libobject/core/utils/data_structure/vector.h>
#include <libobject/core/utils/blob/blob.h>
#include <libobject/core/Map.h>
#include <libobject/concurrent/net/Server.h>

enum {
	BUSD_OBJID,
	BUSD_OBJINFOS,
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

typedef struct busd_object {
    // char *name;
	char *id;
	char  *infos;
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
