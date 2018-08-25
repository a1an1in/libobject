#ifndef __INET_TCP_CLIENT_H__
#define __INET_TCP_CLIENT_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/client/client.h>
#include <libobject/net/socket/inet_tcp_socket.h>

typedef struct inet_tcp_client_s Inet_Tcp_Client;

struct inet_tcp_client_s{
	Client parent;

	int (*construct)(Inet_Tcp_Client *,char *init_str);
	int (*deconstruct)(Inet_Tcp_Client *);
	int (*set)(Inet_Tcp_Client *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*inherited methods from parent*/
    int (*bind)(Inet_Tcp_Client *socket, char *host, char *service);
    int (*connect)(Inet_Tcp_Client *socket, char *host, char *service);
    ssize_t (*send)(Inet_Tcp_Client *socket, const void *buf, size_t len, int flags);
    ssize_t (*recv)(Inet_Tcp_Client *socket, void *buf, size_t len, int flags);
	int (*trustee)(Inet_Tcp_Client *client, struct timeval *tv,
                   void *work_callback, void *opaque);

};

#endif
