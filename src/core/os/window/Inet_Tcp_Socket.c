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
#if (defined(WINDOWS_USER_MODE))
#define _WIN32_WINNT 0x0501

#include <stdio.h>
#include <fcntl.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/utils/registry/registry.h>
#include "Inet_Tcp_Socket.h"

static int __construct(Inet_Tcp_Socket *sk, char *init_str)
{
    SOCKET fd;

    dbg_str(NET_DETAIL, "socket construct, socket addr:%p", sk);

    if (sk->sub_socket_flag == 1) {
        dbg_str(DBG_DETAIL, "socket construct, new sub socket");
        return 1;
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
         dbg_str(NET_ERROR, "create socket error, err:%d",
                 WSAGetLastError());
        return -1;
    }

    sk->parent.fd = fd;

    return 1;
}

static int __deconstrcut(Inet_Tcp_Socket *socket)
{
    dbg_str(NET_DETAIL, "socket deconstruct, socket addr:%p", socket);

    if (socket->parent.fd)
        closesocket(socket->parent.fd);

    return 1;
}

static int __bind(Inet_Tcp_Socket *socket, char *host, char *service)
{
    struct addrinfo  *addr, *addrsave, hint;
    int ret;
    char *h, *s;

    memset(&hint, 0, sizeof(struct addrinfo));
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

    //setsockopt(socket->parent.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (listen(socket->parent.fd, backlog) == -1) {
        perror("listen error");
        return -1;
    }

    return 1;
}

static int __connect(Inet_Tcp_Socket *socket, char *host, char *service)
{
    struct addrinfo  *addr, *addrsave, hint;
    int ret;
    char *h, *s;

    memset(&hint, 0, sizeof(struct addrinfo));
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

static int __accept(Inet_Tcp_Socket *socket, 
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
__sendto(Inet_Tcp_Socket *socket, const void *buf, size_t len,
         int flags, char *remote_host, char *remote_service)
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
    dbg_str(NET_WARNNING, "not supported now");
    return -1;
}

static int __setnonblocking(Inet_Tcp_Socket *socket)
{
    unsigned long mode = 1;

    return ioctlsocket(socket->parent.fd, FIONBIO, (unsigned long *)&mode);
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
    Init_Vfunc_Entry(8 , Inet_Tcp_Socket, connect, __connect),
    Init_Vfunc_Entry(9 , Inet_Tcp_Socket, send, __send),
    Init_Vfunc_Entry(10, Inet_Tcp_Socket, recv, __recv),
    Init_Vfunc_Entry(11, Inet_Tcp_Socket, sendto, __sendto),
    Init_Vfunc_Entry(12, Inet_Tcp_Socket, recvfrom, __recvfrom),
    Init_Vfunc_Entry(13, Inet_Tcp_Socket, getsockopt, __getsockopt),
    Init_Vfunc_Entry(14, Inet_Tcp_Socket, setsockopt, __setsockopt),
    Init_Vfunc_Entry(15, Inet_Tcp_Socket, setnonblocking, __setnonblocking),
    Init_U32___Entry(16, Inet_Tcp_Socket, sub_socket_flag, 0),
    Init_End___Entry(17, Inet_Tcp_Socket),
};
REGISTER_CLASS("Inet_Tcp_Socket", inet_tcp_socket_class_info);

void test_inet_tcp_send()
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();

    char *test_str = "hello world";

    dbg_str(DBG_DETAIL, "run at here");
    socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);

    socket->connect(socket, "127.0.0.1", "11011");
    socket->send(socket, test_str, strlen(test_str), 0);

    object_destroy(socket);
}
REGISTER_TEST_CMD(test_inet_tcp_send);

int test_inet_tcp_recv()
{
    Socket *socket;
    int fd;
    char buf[1024] = {0};
    allocator_t *allocator = allocator_get_default_alloc();

    dbg_str(DBG_DETAIL, "run at here");
    socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);

    dbg_str(NET_DETAIL, "sizeof socket=%d", sizeof(Socket));
    socket->bind(socket, "127.0.0.1", "11011"); 
    socket->listen(socket, 1024);
    fd = socket->accept(socket, NULL, NULL);

    recv(fd, buf, 1024, 0);
    dbg_str(NET_SUC, "recv : %s", buf);

    object_destroy(socket);

    return 0;
}
REGISTER_TEST_CMD(test_inet_tcp_recv);
#endif