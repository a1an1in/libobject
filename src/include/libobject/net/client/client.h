#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/string.h>
#include <libobject/concurrent/worker.h>
#include <libobject/net/socket/socket.h>

typedef struct client_s Client;

struct client_s{
	Obj obj;

	int (*construct)(Client *,char *init_str);
	int (*deconstruct)(Client *);
	int (*set)(Client *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*virtual methods*/
    int (*bind)(Client *client, char *host, char *service);
    int (*connect)(Client *client, char *host, char *service);
    ssize_t (*send)(Client *client, const void *buf, size_t len, int flags);
    ssize_t (*recv)(Client *client, void *buf, size_t len, int flags);
	int (*trustee)(Client *client, struct timeval *tv,
                   void *work_callback, void *opaque);
    
    Worker *worker;
    Socket *socket;
    void *opaque;
};

#define CLIENT_TYPE_INET_TCP "inet_tcp_client_type"
#define CLIENT_TYPE_INET_UDP "inet_udp_client_type"
#define CLIENT_TYPE_UNIX_TCP "unix_tcp_client_type"
#define CLIENT_TYPE_UNIX_UDP "unix_udp_client_type"

void *client(allocator_t *allocator,
             char *type,
             char *host,
             char *service,
             int (*process_task_cb)(void *arg),
             void *opaque);
int client_connect(void *client, char *host, char *service);
int client_send(void *client, void *buf, int len, int flags);
int client_destroy(void *client);

#endif
