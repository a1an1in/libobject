#ifndef __UNIX_UDP_SOCKET_H__
#define __UNIX_UDP_SOCKET_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/socket/socket.h>

typedef struct unix_udp_socket_s Unix_Udp_Socket;

struct unix_udp_socket_s{
	Socket parent;

	int (*construct)(Unix_Udp_Socket *,char *init_str);
	int (*deconstruct)(Unix_Udp_Socket *);
	int (*set)(Unix_Udp_Socket *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*connect)(Unix_Udp_Socket *socket, char *host, char *service);
    int (*bind)(Unix_Udp_Socket *socket, char *host, char *service);
    ssize_t (*send)(Unix_Udp_Socket *socket, const void *buf, size_t len, int flags);
    ssize_t (*write)(Unix_Udp_Socket *socket, const void *buf, size_t len);
    ssize_t (*sendto)(Unix_Udp_Socket *socket, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr,
                      socklen_t addrlen);
    ssize_t (*sendmsg)(Unix_Udp_Socket *socket, const struct msghdr *msg, int flags);
    ssize_t (*read)(Unix_Udp_Socket *socket, void *buf, size_t len);
    ssize_t (*recv)(Unix_Udp_Socket *socket, void *buf, size_t len, int flags);
    ssize_t (*recvfrom)(Unix_Udp_Socket *socket, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, 
                        socklen_t *addrlen);
    ssize_t (*recvmsg)(Unix_Udp_Socket *socket, struct msghdr *msg, int flags);
};

#endif
