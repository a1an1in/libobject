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

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include "Inet_Udp_Socket.h"

static int __construct(Inet_Udp_Socket *udp_socket, char *init_str)
{
    SOCKET sk;

    sk = socket(AF_INET, SOCK_DGRAM, 0);
    udp_socket->parent.fd = sk;
    dbg_str(NET_INFO, "construct windows upd socket:%d", sk);

    return 1;
}

static int __deconstrcut(Inet_Udp_Socket *socket)
{
    closesocket(socket->parent.fd);

    return 1;
}

static int __bind(Inet_Udp_Socket *socket, char *host, char *service)
{
    SOCKADDR_IN socket_addr;
    int port;
    int ret;

    if (service != NULL) {
        port = atoi(service);
    } else {
        port = 0;
    }
    
    socket_addr.sin_addr.S_un.S_addr = inet_addr(host);
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    ret = bind(socket->parent.fd, (SOCKADDR*)&socket_addr, sizeof(SOCKADDR));

    return ret;
}

static int __connect(Inet_Udp_Socket *socket, char *host, char *service)
{
    SOCKADDR_IN socket_addr;
    int port;
    int ret;

    port = atoi(service);
    socket_addr.sin_addr.S_un.S_addr = inet_addr(host);
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    ret = connect(socket->parent.fd, (SOCKADDR*)&socket_addr, sizeof(SOCKADDR));

    return ret;
}

static ssize_t __send(Inet_Udp_Socket *socket, const void *buf, size_t len, int flags)
{
    return send(socket->parent.fd, buf, len, flags);
}

static ssize_t __recv(Inet_Udp_Socket *socket, void *buf, size_t len, int flags)
{
    return recv(socket->parent.fd, buf, len, flags);;
}

static ssize_t 
__sendto(Inet_Udp_Socket *socket, const void *buf, size_t len, int flags, 
         char *host, char *service)
{
    SOCKADDR_IN socket_addr;
    int socket_addr_len;
    int port;
    int ret;

    // 设置服务器地址
    port = atoi(service);
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.S_un.S_addr = inet_addr(host);
    socket_addr.sin_port = htons(port);

    // 向服务器发送数据
    socket_addr_len = sizeof(socket_addr);
    ret = sendto(socket->parent.fd, buf, len, 0, (SOCKADDR *)&socket_addr, socket_addr_len);
    if(ret == SOCKET_ERROR) {
        dbg_str(NET_ERROR, "sendto error");
        return -1;
    }

    return 1;
}

static ssize_t 
__recvfrom(Inet_Udp_Socket *socket, void *buf, size_t len, int flags,
           char *remote_host, int host_len,
           char *remote_service, int service_len)
{
    int ret;
    SOCKADDR_IN socket_addr;
    int socket_addr_len;

    socket_addr_len = sizeof(SOCKADDR);
    ret = recvfrom(socket->parent.fd, buf, len, 0, (SOCKADDR*)&socket_addr, &socket_addr_len);
    if(SOCKET_ERROR == ret) {
        dbg_str(NET_ERROR, "recvfrom error");
        return ret;
    }

    if (remote_host != NULL) {
        strncpy(remote_host, inet_ntoa(socket_addr.sin_addr), host_len);
    }
    if (remote_service != NULL) {
        itoa(socket_addr.sin_port, remote_service, 10);
    }

    return 1;
}

static int __getsockopt(Inet_Udp_Socket *socket, sockoptval *val)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}

static int __setsockopt(Inet_Udp_Socket *socket, sockoptval *val)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}

static int __setnonblocking(Inet_Udp_Socket *socket)
{
    unsigned long mode = 1;

    return ioctlsocket(socket->parent.fd, FIONBIO, (unsigned long *)&mode); 
}

static int __getservice(Inet_Udp_Socket *socket, char *service, size_t service_len) 
{  
    struct sockaddr_storage addr;  
    int addr_len = sizeof(addr);
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

#endif //end of WINDOWS_USER_MODE
