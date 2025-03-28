#ifndef __BUSD_H__
#define __BUSD_H__

#include <libobject/basic_types.h>
#include <libobject/core/utils/data_structure/vector.h>
#include <libobject/core/utils/blob/blob.h>
#include <libobject/core/Map.h>
#include <libobject/core/Struct_Adapter.h>
#include <libobject/concurrent/net/Server.h>

#define BUSD_OBJECT_ID_LEN 41

enum {
	BUSD_OBJID,
	BUSD_OBJINFOS,
	BUSD_INVOKE_METHORD,
    BUSD_INVOKE_ARGC,
    BUSD_INVOKE_ARGS,
    BUSD_STATE,
    BUSD_OPAQUE,
    BUSD_INVOKE_SRC_FD,
    BUSD_TIME,
	__BUSD_MAX
};

typedef struct busd_task_s {
    int fd;
} busd_task_t;

typedef Struct_Adapter Busd_Object_Struct_Adapter;

typedef struct busd_s {
    allocator_t *allocator;
    Server *server;
    char *server_sk_type;
    char *server_host;
    char *server_srv;

    blob_t *blob;
    /*bus 注册的service map。*/
	Map *obj_map;
} busd_t;

typedef struct busd_object {
	char id[BUSD_OBJECT_ID_LEN];
	char *infos;
    int fd;
    allocator_t *allocator;
} busd_object_t;

typedef int (*busd_cmd_callback)(busd_t *busd,  struct blob_attr_s **attr,int fd);
busd_t *busd_create(allocator_t *allocator,
                    char *server_host,
                    char *server_srv,
                    char *socket_type);
int busd_destroy(busd_t *busd);

#endif
