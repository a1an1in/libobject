#ifndef __MY_SOCKET_H__
#define __MY_SOCKET_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/concurrent/work_task.h>

#define DEFAULT_MAX_IP_STR_LEN 64
#define DEFAULT_MAX_PORT_STR_LEN 10

typedef struct socket_s Socket;

typedef struct sockoptval_s {
    uint32_t level;
    uint32_t optname;
    uint32_t value;
} sockoptval;

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
    ssize_t (*send)(Socket *socket, const void *buf, size_t len, int flags);
    ssize_t (*recv)(Socket *socket, void *buf, size_t len, int flags);
    ssize_t (*sendto)(Socket *socket, const void *buf, size_t len, int flags,
            char *remote_host, char *remote_service);
    ssize_t (*recvfrom)(Socket *socket, void *buf, size_t len, int flags,
            char *remote_host, char *remote_service);
    int (*getsockopt)(Socket *socket, sockoptval *val);
    int (*setsockopt)(Socket *socket, sockoptval *val);
    int (*setnonblocking)(Socket *socket);

    int fd;
    char *local_host;
    char *local_service;
    char *remote_host;
    char *remote_service;
};

#endif
