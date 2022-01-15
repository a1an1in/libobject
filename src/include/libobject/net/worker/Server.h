#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/concurrent/Worker.h>
#include <libobject/core/List.h>
#include <libobject/io/Socket.h>

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

#define SERVER_TYPE_INET_TCP "inet_tcp_server_type"
#define SERVER_TYPE_UNIX_TCP "tcp_userver_type"
void *server(allocator_t *allocator, 
             char *type,
             char *host,
             char *service,
             int (*process_task_cb)(void *arg),
             void *opaque);
int server_destroy(void *server);

#endif
