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
#include <fcntl.h> 
#include <sys/un.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Thread.h>
#include "Unix_Udp_Socket.h"

static int __construct(Unix_Udp_Socket *sk, char *init_str)
{
    int skfd;

    if ((skfd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
         dbg_str(NET_ERROR,"create socket error, error_no=%d, err:%s",
                 errno, strerror(errno));
        return -1;
    }

    sk->parent.fd = skfd;

    dbg_str(NET_DETAIL, "unix socket construct, socket addr:%p, skfd=%d", sk, skfd);
    return 0;
}

static int __deconstrcut(Unix_Udp_Socket *socket)
{
    dbg_str(NET_DETAIL, "unix socket deconstruct, socket addr:%p", socket);

    close(socket->parent.fd);

    return 0;
}

static int __bind(Unix_Udp_Socket *socket, char *host, char *service)
{
     struct sockaddr_un un_addr;
     int fd  = socket->parent.fd;
     int ret = 0;

     bzero(&un_addr, sizeof(un_addr));
     un_addr.sun_family = PF_UNIX;
     strcpy(un_addr.sun_path, host);
     unlink(host);

     if ((ret = bind(fd, (struct sockaddr *)&un_addr, sizeof(un_addr))) < 0) {
         dbg_str(NET_ERROR,"fail to bind, error_no=%d, err:%s", ret, strerror (errno));
         ret = -1;
     }                         

     return ret;
}

static int __connect(Unix_Udp_Socket *socket, char *host, char *service)
{
     struct sockaddr_un un_addr;
     int fd  = socket->parent.fd;
     int ret = 0;

     bzero(&un_addr, sizeof(un_addr));
     un_addr.sun_family = PF_UNIX;
     strcpy(un_addr.sun_path, host);

     if ((ret = connect(fd, (struct sockaddr *)&un_addr, sizeof(un_addr))) < 0) {
         dbg_str(NET_ERROR,"unix udp, fail to connect, error_no=%d", ret);
         ret = -1;
     }                         

     return ret;
}

static ssize_t __send(Unix_Udp_Socket *socket, const void *buf, size_t len, int flags)
{
    int ret;

    ret = send(socket->parent.fd, buf, len, flags);

    if (ret <= 0) {
        dbg_str(NET_ERROR, "send error: %s", strerror(errno));
    }

    return ret;
}

static ssize_t __recv(Unix_Udp_Socket *socket, void *buf, size_t len, int flags)
{
    return recv(socket->parent.fd, buf, len, flags);
}

static ssize_t 
__sendto(Unix_Udp_Socket *socket, const void *buf, size_t len,
         int flags, const struct sockaddr *dest_addr, 
         socklen_t addrlen)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}


static ssize_t 
__recvfrom(Unix_Udp_Socket *socket, void *buf, size_t len, int flags, 
           char *remote_host, int host_len,
           char *remote_service, int service_len)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}

static int __getsockopt(Unix_Udp_Socket *socket, sockoptval *val)
{
    dbg_str(NET_DETAIL, "not supported now");
}

static int __setsockopt(Unix_Udp_Socket *socket, sockoptval *val)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
}

static int __setnonblocking(Unix_Udp_Socket *socket)
{
    if (fcntl(socket->parent.fd, F_SETFL, (fcntl(socket->parent.fd, F_GETFD, 0) | O_NONBLOCK)) == -1) {
        return -1;
    }                     

    return 0;
}

static class_info_entry_t inet_udp_socket_class_info[] = {
    Init_Obj___Entry(0 , Socket, parent),
    Init_Nfunc_Entry(1 , Unix_Udp_Socket, construct, __construct),
    Init_Nfunc_Entry(2 , Unix_Udp_Socket, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Unix_Udp_Socket, set, NULL),
    Init_Vfunc_Entry(4 , Unix_Udp_Socket, get, NULL),
    Init_Vfunc_Entry(5 , Unix_Udp_Socket, bind, __bind),
    Init_Vfunc_Entry(6 , Unix_Udp_Socket, connect, __connect),
    Init_Vfunc_Entry(7 , Unix_Udp_Socket, send, __send),
    Init_Vfunc_Entry(8 , Unix_Udp_Socket, recv, __recv),
    Init_Vfunc_Entry(9 , Unix_Udp_Socket, sendto, __sendto),
    Init_Vfunc_Entry(10, Unix_Udp_Socket, recvfrom, __recvfrom),
    Init_Vfunc_Entry(11, Unix_Udp_Socket, getsockopt, __getsockopt),
    Init_Vfunc_Entry(12, Unix_Udp_Socket, setsockopt, __setsockopt),
    Init_Vfunc_Entry(13, Unix_Udp_Socket, setnonblocking, __setnonblocking),
    Init_End___Entry(14, Unix_Udp_Socket),
};
REGISTER_CLASS("Unix_Udp_Socket", inet_udp_socket_class_info);

#if 0
#define UNIX_CLIENT_PATH "/tmp/unix_client_01"
#define UNIX_SERVER_PATH "/tmp/unix_server_01"

void test_unix_udp_socket_send()
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_instance();

    char *test_str = "hello world";

    socket = OBJECT_NEW(allocator, Unix_Udp_Socket, NULL);

    socket->connect(socket, UNIX_SERVER_PATH, NULL);
    socket->send(socket, test_str, strlen(test_str), 0);

    while(1) sleep(1);

    object_destroy(socket);
}
REGISTER_TEST_CMD(test_unix_udp_socket_send);

void test_unix_udp_socket_recv()
{
    Socket *socket;
    char buf[1024] = {0};
    allocator_t *allocator = allocator_get_default_instance();

    socket = OBJECT_NEW(allocator, Unix_Udp_Socket, NULL);

    socket->bind(socket, UNIX_SERVER_PATH, NULL); 
    /*
     *socket->setsockopt(socket, 0, 0, NULL);
     */

    while(1) {
        socket->recv(socket, buf, 1024, 0);
        dbg_str(NET_SUC, "recv : %s", buf);
    }

    object_destroy(socket);
}
REGISTER_TEST_CMD(test_unix_udp_socket_recv);
#endif
#endif
