#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/concurrent/Worker.h>
#include <libobject/core/List.h>
#include <libobject/core/io/Socket.h>

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
    int (*close_subsocket)(Server *server, Socket *socket);

    Worker *worker;
    Socket *socket;
    void *opaque;
    List *working_workers;
    List *leisure_workers;
};

#endif
