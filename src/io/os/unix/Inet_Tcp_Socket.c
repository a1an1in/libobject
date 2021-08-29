/**
 * @file Socket_Udp.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-28
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) || defined(IOS_USER_MODE) || defined(MAC_USER_MODE))

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h> 
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/Thread.h>
#include <libobject/core/utils/registry/registry.h>
#include "Inet_Tcp_Socket.h"

static int __get_sockoptval_size(int optname)
{
    int size = 0;

    switch(optname) {
        case SO_REUSEPORT:
        case SO_KEEPALIVE:
        case SO_REUSEADDR:
            size = sizeof(int);
            break;
        default:
            size = -1;
    }

    return size;
}

static int __construct(Inet_Tcp_Socket *sk, char *init_str)
{
    int skfd;

    dbg_str(NET_DETAIL, "socket construct, socket addr:%p", sk);

    if ((skfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
         dbg_str(NET_ERROR, "create socket error, error_no=%d, err:%s",
                 errno, strerror(errno));
        return -1;
    }

    sk->parent.fd = skfd;

    return 0;
}

static int __deconstrcut(Inet_Tcp_Socket *socket)
{
    dbg_str(NET_DETAIL, "socket deconstruct, socket addr:%p", socket);

    if (socket->parent.fd)
        close(socket->parent.fd);

    return 0;
}

static int __bind(Inet_Tcp_Socket *socket, char *host, char *service)
{
    struct addrinfo  *addr, *addrsave, hint;
    int skfd, ret;
    char *h, *s;

    bzero(&hint, sizeof(hint));
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_DGRAM;

    if (host == NULL && service == NULL) {
        h = socket->parent.local_host;
        s = socket->parent.local_service;
    } else {
        h = host;
        s = service;
    }

    if ((ret = getaddrinfo(h, s, &hint, &addr)) != 0){
        dbg_str(DBG_ERROR, "getaddrinfo error: %s", gai_strerror(ret));
        return -1;
    }
    addrsave = addr;

    if (addr != NULL) {
        dbg_str(NET_DETAIL, "ai_family=%d type=%d", addr->ai_family, addr->ai_socktype);
    } else {
        dbg_str(DBG_ERROR, "getaddrinfo err");
        return -1;
    }                      

    do {
        if ((ret = bind(socket->parent.fd, addr->ai_addr, addr->ai_addrlen)) == 0)
            break;
    } while ((addr = addr->ai_next) != NULL);

    if (addr == NULL) {
        dbg_str(NET_WARNNING, "bind error for %s %s", host, service);
    }

    freeaddrinfo(addrsave);

    return ret;
}

static int __listen(Inet_Tcp_Socket *socket, int backlog)
{
    int opt = 1;

    setsockopt(socket->parent.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (listen(socket->parent.fd, backlog) == -1) {
        perror("listen error");
        return -1;
    }

    return 0;
}

static int __connect(Inet_Tcp_Socket *socket, char *host, char *service)
{
    struct addrinfo  *addr, *addrsave, hint;
    int skfd, ret;
    char *h, *s;

    bzero(&hint, sizeof(hint));
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_DGRAM;

    if (host == NULL && service == NULL) {
        h = socket->parent.remote_host;
        s = socket->parent.remote_service;
    } else {
        h = host;
        s = service;
    }

    if ((ret = getaddrinfo(h, s, &hint, &addr)) != 0){
        dbg_str(DBG_ERROR, "getaddrinfo error: %s", gai_strerror(ret));
        return -1;
    }
    addrsave = addr;

    if (addr != NULL) {
        dbg_str(NET_DETAIL, "ai_family=%d type=%d", addr->ai_family, addr->ai_socktype);
    } else {
        dbg_str(DBG_ERROR, "getaddrinfo err");
        return -1;
    }                      

    do {
        if ((ret = connect(socket->parent.fd, addr->ai_addr, addr->ai_addrlen)) == 0)
            break;
    } while ((addr = addr->ai_next) != NULL);

    if (addr == NULL) {
        dbg_str(NET_ERROR, "connect error for %s %s", host, service);
    }

    freeaddrinfo(addrsave);

    return ret;
}

static Socket * __accept(Inet_Tcp_Socket *socket, 
                         char *remote_host, char *remote_service)
{
    struct sockaddr_storage cliaddr;
    socklen_t len;
    allocator_t *allocator = socket->parent.obj.allocator;
    int connfd;
    Socket *ret = NULL;


    connfd = accept(socket->parent.fd, (struct sockaddr *)&cliaddr, &len);

    if (connfd > 0) {
        ret = object_new(allocator, "Inet_Tcp_Socket", NULL);//in order to close fd
        dbg_str(DBG_DETAIL, "server accept new fd:%d", connfd);
        ret->fd = connfd;
    } 

    return ret;
}

static int __accept_fd(Inet_Tcp_Socket *socket, 
                       char *remote_host, char *remote_service)
{
    struct sockaddr_storage cliaddr;
    socklen_t len;
    allocator_t *allocator = socket->parent.obj.allocator;

    return accept(socket->parent.fd, (struct sockaddr *)&cliaddr, &len);
}

static ssize_t __send(Inet_Tcp_Socket *socket, const void *buf, size_t len, int flags)
{
    int ret;

    ret = send(socket->parent.fd, buf, len, flags);

    if (ret <= 0) {
        dbg_str(NET_ERROR, "send error: %s", strerror(errno));
    }

    return ret;
}

static ssize_t __recv(Inet_Tcp_Socket *socket, void *buf, size_t len, int flags)
{
    return recv(socket->parent.fd, buf, len, flags);
}

static ssize_t 
__sendto(Inet_Tcp_Socket *socket, const void *buf, size_t len, int flags, 
         char *remote_host, char *remote_service)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}

static ssize_t 
__recvfrom(Inet_Tcp_Socket *socket, void *buf, size_t len, int flags, 
           char *remote_host, int host_len,
           char *remote_service, int service_len)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}

static int __getsockopt(Inet_Tcp_Socket *socket, sockoptval *val)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}

static int __setsockopt(Inet_Tcp_Socket *socket, sockoptval *val)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;

#if 0
    int size;

    dbg_str(NET_DETAIL, "setsockopt level=%d, optname=%d", level, optname);

    size = __get_sockoptval_size(optname);
    if (size <= 0) {
        dbg_str(NET_WARNNING, "setsockopt level=%d, optname=%d, not supported now", 
                level, optname);
        return -1;
    }

    return setsockopt(socket->parent.fd, level, optname, val, size);
#endif
}

static int __setnonblocking(Inet_Tcp_Socket *socket)
{
    int fd = socket->parent.fd;

    if (fcntl(fd, F_SETFL, (fcntl(fd, F_GETFD, 0) | O_NONBLOCK)) == -1) {
        return -1;
    }                     

    return 1;
}

static class_info_entry_t inet_tcp_socket_class_info[] = {
    Init_Obj___Entry(0 , Socket, parent),
    Init_Nfunc_Entry(1 , Inet_Tcp_Socket, construct, __construct),
    Init_Nfunc_Entry(2 , Inet_Tcp_Socket, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Inet_Tcp_Socket, set, NULL),
    Init_Vfunc_Entry(4 , Inet_Tcp_Socket, get, NULL),
    Init_Vfunc_Entry(5 , Inet_Tcp_Socket, bind, __bind),
    Init_Vfunc_Entry(6 , Inet_Tcp_Socket, listen, __listen),
    Init_Vfunc_Entry(7 , Inet_Tcp_Socket, accept, __accept),
    Init_Vfunc_Entry(8 , Inet_Tcp_Socket, accept_fd, __accept_fd),
    Init_Vfunc_Entry(9 , Inet_Tcp_Socket, connect, __connect),
    Init_Vfunc_Entry(10, Inet_Tcp_Socket, send, __send),
    Init_Vfunc_Entry(11, Inet_Tcp_Socket, recv, __recv),
    Init_Vfunc_Entry(12, Inet_Tcp_Socket, sendto, __sendto),
    Init_Vfunc_Entry(13, Inet_Tcp_Socket, recvfrom, __recvfrom),
    Init_Vfunc_Entry(14, Inet_Tcp_Socket, getsockopt, __getsockopt),
    Init_Vfunc_Entry(15, Inet_Tcp_Socket, setsockopt, __setsockopt),
    Init_Vfunc_Entry(16, Inet_Tcp_Socket, setnonblocking, __setnonblocking),
    Init_End___Entry(17, Inet_Tcp_Socket),
};
REGISTER_CLASS("Inet_Tcp_Socket", inet_tcp_socket_class_info);

void test_inet_tcp_socket_send()
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();

    char *test_str = "hello world";

    dbg_str(DBG_DETAIL, "run at here");
    socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);

    socket->connect(socket, "127.0.0.1", "11011");
    socket->send(socket, test_str, strlen(test_str), 0);

    while(1) sleep(1);

    object_destroy(socket);
}
REGISTER_TEST_FUNC(test_inet_tcp_socket_send);

int test_inet_tcp_socket_recv()
{
    Socket *socket, *new;
    char buf[1024] = {0};
    allocator_t *allocator = allocator_get_default_alloc();

    dbg_str(DBG_DETAIL, "run at here");
    socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);

    dbg_str(NET_DETAIL, "sizeof socket=%d", sizeof(Socket));
    socket->bind(socket, "127.0.0.1", "11011"); 
    socket->listen(socket, 1024);
    new = socket->accept(socket, NULL, NULL);

    new->recv(new, buf, 1024, 0);
    dbg_str(NET_SUC, "recv : %s", buf);

    sleep(10);

    object_destroy(new);
    object_destroy(socket);

    return 0;
}
REGISTER_TEST_FUNC(test_inet_tcp_socket_recv);
#endif
