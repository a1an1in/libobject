#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/net/net_task.h>

#define DEFAULT_MAX_IP_STR_LEN 64
#define DEFAULT_MAX_PORT_STR_LEN 10

typedef struct socket_s Socket;
typedef union sockoptval_u{
    int int_val;
    long long_val;
    struct linger linger_val;
    struct timeval timeval_val;
}sockoptval;

struct socket_s{
	Obj obj;

	int (*construct)(Socket *,char *init_str);
	int (*deconstruct)(Socket *);
	int (*set)(Socket *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*listen)(Socket *socket, int backlog);
    Socket * (*accept)(Socket *socket, char *remote_host, char *remote_service);
    int (*accept_fd)(Socket *socket, char *remote_host, char *remote_service);
    int (*connect)(Socket *socket, char *host, char *service);
    int (*bind)(Socket *socket, char *host, char *service);
    ssize_t (*write)(Socket *socket, const void *buf, size_t len);
    ssize_t (*send)(Socket *socket, const void *buf, size_t len, int flags);
    ssize_t (*sendto)(Socket *socket, 
                      const void *buf, 
                      size_t len,
                      int flags,
                      const struct sockaddr *dest_addr,
                      socklen_t addrlen);
    ssize_t (*sendmsg)(Socket *socket, const struct msghdr *msg, int flags);
    ssize_t (*read)(Socket *socket, void *buf, size_t len);
    ssize_t (*recv)(Socket *socket, void *buf, size_t len, int flags);
    ssize_t (*recvfrom)(Socket *socket, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, 
                        socklen_t *addrlen);
    ssize_t (*recvmsg)(Socket *socket, struct msghdr *msg, int flags);
    int (*getsockopt)(Socket *socket, int level, int optname, sockoptval *val);
    int (*setsockopt)(Socket *socket, int level, int optname, sockoptval *val);
    int (*setnonblocking)(Socket *socket);

    //-----------------------------------------------
    void (*setblock)(Socket *,int isblock);
    int  (*close)(Socket * socket);
    int  (*shutdown)(Socket *socket,int ihow);
    void (*setnoclosewait)(Socket *);

    void (*setclosewait)(Socket *,int delaytime);
    void (*setclosewaitdefault)(Socket *);
    void (*set_timeout)(Socket *,int timeout);
    int  (*get_timeout)(Socket *);
    int  (*get_socketfd)(Socket*);
    
    int fd;
    int socket_type;
    int isowner;
    int timeout;

    char *local_host;
    char *local_service;
    char *remote_host;
    char *remote_service;
};

#endif
