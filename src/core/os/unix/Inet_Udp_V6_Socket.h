#ifndef __INET_UDP_V6_SOCKET_H__
#define __INET_UDP_V6_SOCKET_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/io/Socket.h>

typedef struct inet_udp_v6_socket_s Inet_Udp_V6_Socket;

struct inet_udp_v6_socket_s{
	Socket parent;

	int (*construct)(Inet_Udp_V6_Socket *,char *init_str);
	int (*deconstruct)(Inet_Udp_V6_Socket *);
	int (*set)(Inet_Udp_V6_Socket *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*connect)(Inet_Udp_V6_Socket *socket, char *host, char *service);
    int (*bind)(Inet_Udp_V6_Socket *socket, char *host, char *service);
    ssize_t (*send)(Inet_Udp_V6_Socket *socket, const void *buf, size_t len, int flags);
    ssize_t (*recv)(Inet_Udp_V6_Socket *socket, void *buf, size_t len, int flags);
    ssize_t (*sendto)(Inet_Udp_V6_Socket *socket, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr,
                      socklen_t addrlen);
    ssize_t (*recvfrom)(Inet_Udp_V6_Socket *socket, void *buf, size_t len, int flags,
                        char *remote_host, int host_len,
                        char *remote_service, int service_len);
    int (*getsockopt)(Inet_Udp_V6_Socket *socket, int level, int optname, sockoptval *val);
    int (*setsockopt)(Inet_Udp_V6_Socket *socket, int level, int optname, sockoptval *val);
    int (*setnonblocking)(Inet_Udp_V6_Socket *socket);
};

#endif
