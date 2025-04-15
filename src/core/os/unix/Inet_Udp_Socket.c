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

#if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) || defined(ANDROID_USER_MODE) || defined(IOS_USER_MODE) || defined(MAC_USER_MODE))

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h> 
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Thread.h>
#include "Inet_Udp_Socket.h"

static int __construct(Inet_Udp_Socket *sk, char *init_str)
{
    int skfd;


    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
         dbg_str(NET_ERROR, "create socket error, error_no=%d, err:%s",
                 errno, strerror(errno));
        return -1;
    }

    sk->parent.fd = skfd;

    dbg_str(NET_DETAIL, "socket construct, socket addr:%p, skfd=%d", sk, skfd);

    return 0;
}

static int __deconstrcut(Inet_Udp_Socket *socket)
{
    dbg_str(NET_DETAIL, "socket deconstruct, socket addr:%p", socket);

    close(socket->parent.fd);

    return 0;
}

static int __bind(Inet_Udp_Socket *socket, char *host, char *service)
{
    struct addrinfo  *addr, *addrsave, hint;
    int skfd, ret, port;
    char *h, *s;

    bzero(&hint, sizeof(hint));
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_DGRAM;

    if (host == NULL) {
        h = "127.0.0.1";
    } else {
        h = host;
    }

    if (service == NULL) {
        hint.ai_flags = AI_PASSIVE; 
        s = "0";
    } else {
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
        dbg_str(NET_WARN, "bind error for %s %s", host, service);
    }

    freeaddrinfo(addrsave);

    return ret;
}

static int __connect(Inet_Udp_Socket *socket, char *host, char *service)
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

static ssize_t __write(Inet_Udp_Socket *socket, const void *buf, size_t len)
{
    dbg_str(NET_DETAIL, "socket write");

    return write(socket->parent.fd, buf, len);
}

static ssize_t __send(Inet_Udp_Socket *socket, const void *buf, size_t len, int flags)
{
    int ret;

    ret = send(socket->parent.fd, buf, len, flags);

    if (ret <= 0) {
        dbg_str(NET_ERROR, "send error: %s", strerror(errno));
    }

    return ret;
}

static ssize_t __recv(Inet_Udp_Socket *socket, void *buf, size_t len, int flags)
{
    return recv(socket->parent.fd, buf, len, flags);
}

static ssize_t 
__sendto(Inet_Udp_Socket *socket, const void *buf, size_t len, int flags,
         char *host, char *service)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}

static ssize_t 
__recvfrom(Inet_Udp_Socket *socket, void *buf, size_t len, int flags, 
           char *remote_host, int host_len,
           char *remote_service, int service_len)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}

static int __getsockopt(Inet_Udp_Socket *socket, sockoptval *val)
{
    dbg_str(NET_DETAIL, "not supported now");
}

static int __setsockopt(Inet_Udp_Socket *socket, sockoptval *val)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}

static int __setnonblocking(Inet_Udp_Socket *socket)
{
    if (fcntl(socket->parent.fd, F_SETFL, (fcntl(socket->parent.fd, F_GETFD, 0) | O_NONBLOCK)) == -1) {
        return -1;
    }                     

    return 0;
}

static int __getservice(Inet_Udp_Socket *socket, char *service, size_t service_len) 
{  
    struct sockaddr_storage addr;  
    socklen_t addr_len = sizeof(addr);
    uint16_t port;
    int ret, written;

    TRY {
        // 获取套接字地址信息  
        if (getsockname(socket->parent.fd, (struct sockaddr*)&addr, &addr_len) != 0) {  
            THROW(-1); 
        }  

        // 提取端口号  
        if (addr.ss_family == AF_INET) {  
            struct sockaddr_in *ipv4_addr = (struct sockaddr_in*)&addr;  
            port = ntohs(ipv4_addr->sin_port);  
        } else if (addr.ss_family == AF_INET6) {  
            struct sockaddr_in6 *ipv6_addr = (struct sockaddr_in6*)&addr;  
            port = ntohs(ipv6_addr->sin6_port);  
        } else {  
            THROW(-1);
        }  

        // 将端口号转换为字符串  
        written = snprintf(service, service_len, "%d", port);  
        if (written < 0 || written >= service_len) {  
            THROW(-1);
        }
    } CATCH (ret) {}

    return ret; 
}  

static class_info_entry_t inet_udp_socket_class_info[] = {
    Init_Obj___Entry(0 , Socket, parent),
    Init_Nfunc_Entry(1 , Inet_Udp_Socket, construct, __construct),
    Init_Nfunc_Entry(2 , Inet_Udp_Socket, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Inet_Udp_Socket, set, NULL),
    Init_Vfunc_Entry(4 , Inet_Udp_Socket, get, NULL),
    Init_Vfunc_Entry(5 , Inet_Udp_Socket, bind, __bind),
    Init_Vfunc_Entry(6 , Inet_Udp_Socket, connect, __connect),
    Init_Vfunc_Entry(7 , Inet_Udp_Socket, send, __send),
    Init_Vfunc_Entry(8 , Inet_Udp_Socket, recv, __recv),
    Init_Vfunc_Entry(9 , Inet_Udp_Socket, sendto, __sendto),
    Init_Vfunc_Entry(10, Inet_Udp_Socket, recvfrom, __recvfrom),
    Init_Vfunc_Entry(11, Inet_Udp_Socket, getsockopt, __getsockopt),
    Init_Vfunc_Entry(12, Inet_Udp_Socket, setsockopt, __setsockopt),
    Init_Vfunc_Entry(13, Inet_Udp_Socket, setnonblocking, __setnonblocking),
    Init_Vfunc_Entry(14, Inet_Udp_Socket, getservice, __getservice),
    Init_End___Entry(15, Inet_Udp_Socket),
};
REGISTER_CLASS(Inet_Udp_Socket, inet_udp_socket_class_info);

#if 0
void test_inet_udp_socket_send()
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_instance();

    char *test_str = "hello world";

    socket = OBJECT_NEW(allocator, Inet_Udp_Socket, NULL);

    socket->connect(socket, "127.0.0.1", "11011");
    socket->send(socket, test_str, strlen(test_str), 0);

    while(1) sleep(1);

    object_destroy(socket);
}
REGISTER_TEST_CMD(test_inet_udp_socket_send);

void test_inet_udp_socket_recv()
{
    Socket *socket;
    char buf[1024] = {0};
    allocator_t *allocator = allocator_get_default_instance();

    /*
     *dbg_str(NET_DETAIL, "run at here");
     */
    socket = OBJECT_NEW(allocator, Inet_Udp_Socket, NULL);

    socket->bind(socket, "127.0.0.1", "11011"); 
    socket->setsockopt(socket, NULL);

    while(1) {
        socket->recv(socket, buf, 1024, 0);
        dbg_str(NET_SUC, "recv : %s", buf);
    }

    object_destroy(socket);
}
REGISTER_TEST_CMD(test_inet_udp_socket_recv);
#endif
#endif
