#ifndef __INET_UDP_SOCKET_H__
#define __INET_UDP_SOCKET_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/socket/socket.h>

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
    ssize_t (*write)(Inet_Udp_Socket *socket, const void *buf, size_t len);
    ssize_t (*sendto)(Inet_Udp_Socket *socket, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr,
                      socklen_t addrlen);
    ssize_t (*sendmsg)(Inet_Udp_Socket *socket, const struct msghdr *msg, int flags);
    ssize_t (*read)(Inet_Udp_Socket *socket, void *buf, size_t len);
    ssize_t (*recv)(Inet_Udp_Socket *socket, void *buf, size_t len, int flags);
    ssize_t (*recvfrom)(Inet_Udp_Socket *socket, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, 
                        socklen_t *addrlen);
    ssize_t (*recvmsg)(Inet_Udp_Socket *socket, struct msghdr *msg, int flags);
};

#endif
