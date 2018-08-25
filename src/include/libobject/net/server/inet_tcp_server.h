#ifndef __INET_TCP_SERVER_H__
#define __INET_TCP_SERVER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/server/server.h>
#include <libobject/net/socket/inet_tcp_socket.h>

typedef struct inet_tcp_server_s Inet_Tcp_Server;

struct inet_tcp_server_s{
	Server parent;

	int (*construct)(Inet_Tcp_Server *,char *init_str);
	int (*deconstruct)(Inet_Tcp_Server *);
	int (*set)(Inet_Tcp_Server *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*inherited methods*/
    int (*bind)(Inet_Tcp_Server *server, char *host, char *service);
	int (*trustee)(Inet_Tcp_Server *server, void *work_callback, void *opaque);

};

#endif
