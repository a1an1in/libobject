#ifndef __UDP_SOCKET_H__
#define __UDP_SOCKET_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/socket.h>

typedef struct udp_socket_s Udp_Socket;

struct udp_socket_s{
	Socket parent;

	int (*construct)(Udp_Socket *,char *init_str);
	int (*deconstruct)(Udp_Socket *);
	int (*set)(Udp_Socket *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*connect)(Udp_Socket *socket, char *host, char *service);
    int (*bind)(Udp_Socket *socket, char *host, char *service);
    ssize_t (*send)(Udp_Socket *socket, const void *buf, size_t len, int flags);
    ssize_t (*write)(Udp_Socket *socket, const void *buf, size_t len);
    ssize_t (*sendto)(Udp_Socket *socket, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr,
                      socklen_t addrlen);
    ssize_t (*sendmsg)(Udp_Socket *socket, const struct msghdr *msg, int flags);
    ssize_t (*read)(Udp_Socket *socket, void *buf, size_t len);
    ssize_t (*recv)(Udp_Socket *socket, void *buf, size_t len, int flags);
    ssize_t (*recvfrom)(Udp_Socket *socket, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, 
                        socklen_t *addrlen);
    ssize_t (*recvmsg)(Udp_Socket *socket, struct msghdr *msg, int flags);

    int fd;
#define MAX_IP_STR_LEN 64
#define MAX_PORT_STR_LEN 10
    char local_host[MAX_IP_STR_LEN];
    char local_service[MAX_PORT_STR_LEN];
    char remote_host[MAX_IP_STR_LEN];
    char remote_service[MAX_PORT_STR_LEN];
#undef MAX_IP_STR_LEN
#undef MAX_PORT_STR_LEN
};

#endif
