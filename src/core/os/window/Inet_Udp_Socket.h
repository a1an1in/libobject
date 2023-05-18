#ifndef __INET_UDP_SOCKET_H__
#define __INET_UDP_SOCKET_H__

#include <stdio.h>
#include <windows.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/io/Socket.h>

typedef struct inet_udp_socket_s Inet_Udp_Socket;

struct inet_udp_socket_s{
	Socket parent;

	int (*construct)(Inet_Udp_Socket *,char *init_str);
	int (*deconstruct)(Inet_Udp_Socket *);
	int (*set)(Inet_Udp_Socket *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*connect)(Inet_Udp_Socket *socket, char *host, char *service);
    int (*bind)(Inet_Udp_Socket *socket, char *host, char *service);
    ssize_t (*send)(Inet_Udp_Socket *socket, const void *buf, size_t len, int flags);
    ssize_t (*recv)(Inet_Udp_Socket *socket, void *buf, size_t len, int flags);
    ssize_t (*sendto)(Inet_Udp_Socket *socket, const void *buf, size_t len, int flags,
                      char *host, char *service);
    ssize_t (*recvfrom)(Inet_Udp_Socket *socket, void *buf, size_t len, int flags,
                        char *remote_host, int host_len,
                        char *remote_service, int service_len);
    int (*getsockopt)(Inet_Udp_Socket *socket, int level, int optname, sockoptval *val);
    int (*setsockopt)(Inet_Udp_Socket *socket, int level, int optname, sockoptval *val);
    int (*setnonblocking)(Inet_Udp_Socket *socket);
};

#endif
