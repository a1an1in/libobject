#ifndef __UNIX_UDP_SOCKET_H__
#define __UNIX_UDP_SOCKET_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/io/Socket.h>

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
    ssize_t (*recv)(Unix_Udp_Socket *socket, void *buf, size_t len, int flags);
    ssize_t (*sendto)(Unix_Udp_Socket *socket, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr,
                      socklen_t addrlen);
    ssize_t (*recvfrom)(Unix_Udp_Socket *socket, void *buf, size_t len, int flags,
                        char *remote_host, int host_len,
                        char *remote_service, int service_len);
    int (*getsockopt)(Unix_Udp_Socket *socket, int level, int optname, sockoptval *val);
    int (*setsockopt)(Unix_Udp_Socket *socket, int level, int optname, sockoptval *val);
    int (*setnonblocking)(Unix_Udp_Socket *socket);
};

#endif
