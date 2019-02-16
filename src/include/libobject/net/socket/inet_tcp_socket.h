#ifndef __INET_TCP_SOCKET_H__
#define __INET_TCP_SOCKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/socket/socket.h>

#define RECV_BUFFER_LEN 1024



typedef struct inet_tcp_socket_s Inet_Tcp_Socket;

struct inet_tcp_socket_s{
	Socket parent;

	int (*construct)(Inet_Tcp_Socket *,char *init_str);
	int (*deconstruct)(Inet_Tcp_Socket *);
	int (*set)(Inet_Tcp_Socket *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    Socket * (*accept)(Inet_Tcp_Socket *socket, char *remote_host, char *remote_service);
    int (*accept_fd)(Inet_Tcp_Socket *socket, char *remote_host, char *remote_service);

    /*inherit metheods from parent*/
    int (*listen)(Inet_Tcp_Socket *socket, int backlog);
    int (*connect)(Inet_Tcp_Socket *socket, char *host, char *service);
    int (*bind)(Inet_Tcp_Socket *socket, char *host, char *service);
    ssize_t (*send)(Inet_Tcp_Socket *socket, const void *buf, size_t len, int flags);
    ssize_t (*write)(Inet_Tcp_Socket *socket, const void *buf, size_t len);
    ssize_t (*sendto)(Inet_Tcp_Socket *socket, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr,
                      socklen_t addrlen);
    ssize_t (*sendmsg)(Inet_Tcp_Socket *socket, const struct msghdr *msg, int flags);
    ssize_t (*read)(Inet_Tcp_Socket *socket, void *buf, size_t len);
    ssize_t (*recv)(Inet_Tcp_Socket *socket, void *buf, size_t len, int flags);
    ssize_t (*recvfrom)(Inet_Tcp_Socket *socket, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, 
                        socklen_t *addrlen);
    ssize_t (*recvmsg)(Inet_Tcp_Socket *socket, struct msghdr *msg, int flags);
    
    
    void (*setblock)(Inet_Tcp_Socket *,int isblock);
    int  (*close)(Inet_Tcp_Socket * socket);
    int  (*shutdown)(Inet_Tcp_Socket *socket,int ihow);
    void (*setnoclosewait)(Inet_Tcp_Socket *);

    void (*setclosewait)(Inet_Tcp_Socket *,int delaytime);
    void (*setclosewaitdefault)(Inet_Tcp_Socket *);
    void (*set_timeout)(Inet_Tcp_Socket *,int timeout);
    int  (*get_timeout)(Inet_Tcp_Socket *);

    int (*get_socketfd)(Inet_Tcp_Socket *);
    /*attr value*/
    
    int timeout;
};

#endif
