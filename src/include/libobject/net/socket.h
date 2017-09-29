#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/obj.h>


typedef struct socket_s Socket;

struct socket_s{
	Obj obj;

	int (*construct)(Socket *,char *init_str);
	int (*deconstruct)(Socket *);
	int (*set)(Socket *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*connect)(Socket *socket, const struct sockaddr *addr, socklen_t addrlen);
    int (*bind)(Socket *socket, const struct sockaddr *addr, socklen_t addrlen);
    ssize_t (*write)(Socket *socket, const void *buf, size_t len);
    ssize_t (*send)(Socket *socket, const void *buf, size_t len, int flags);
    ssize_t (*sendto)(Socket *socket, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr,
                      socklen_t addrlen);
    ssize_t (*sendmsg)(Socket *socket, const struct msghdr *msg, int flags);
    ssize_t (*read)(Socket *socket, const void *buf, size_t len);
    ssize_t (*recv)(Socket *socket, void *buf, size_t len, int flags);
    ssize_t (*recvfrom)(Socket *socket, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, 
                        socklen_t *addrlen);
    ssize_t (*recvmsg)(Socket *socket, struct msghdr *msg, int flags);

};

#endif
