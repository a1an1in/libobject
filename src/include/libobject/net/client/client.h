#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/concurrent/worker.h>
#include <libobject/net/socket.h>

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

#endif
