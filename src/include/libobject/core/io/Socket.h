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
    int (*accept)(Socket *socket, char *remote_host, char *remote_service);
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
    int (*getservice)(Socket *socket, char *service, int len);

    /* 
     * keep_alive可以是双向的，即客户端可以主动给服务器发，或服务器主动给客户端发送。在使能了SO_KEEPALIVE后，即启用了保活机制。
     * keep_idle, 当客户端与服务器没有交互数据达到TCP_KEEPIDLE的空闲时间后，TCP将会给对方发送探测包。
     * keep_intvl, 如果上一次的探测包没有得到响应，那么将用TCP_KEEPINTVL作为下一次的探测包间隔。
     * keep_cnt, 当连续发送了TCP_KEEPCNT次数的探测包都未收到响应后，本地将会释放当前连接资源，并且通知应用层连接断开。
     */
    int (*setkeepalive)(Socket *socket, int keep_alive, int keep_idle, int keep_intvl, int keep_cnt);

    int fd;
    char *local_host;
    char *local_service;
    char *remote_host;
    char *remote_service;
    void *opaque;
};

#endif
