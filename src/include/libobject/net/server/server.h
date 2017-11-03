#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/socket/socket.h>
#include <libobject/concurrent/worker.h>
#include <libobject/core/list.h>

typedef struct server_s Server;

struct server_s{
	Obj obj;

	int (*construct)(Server *,char *init_str);
	int (*deconstruct)(Server *);
	int (*set)(Server *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*bind)(Server *server, char *host, char *service);
	int (*trustee)(Server *server, void *work_callback, void *opaque);

    Worker *worker;
    Socket *socket;
    void *opaque;
    List *workers;
};

#endif
