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
#include <fcntl.h> 
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/event/Event_Base.h>
#include "Inet_Udp_Socket.h"
#include <libobject/core/utils/registry/registry.h>

static int __construct(Inet_Udp_Socket *sk, char *init_str)
{
    dbg_str(NET_WARNNING, "not supported now");
    return -1;
}

static int __deconstrcut(Inet_Udp_Socket *socket)
{
    dbg_str(NET_WARNNING, "not supported now");
    return -1;
}

static int __bind(Inet_Udp_Socket *socket, char *host, char *service)
{
    dbg_str(NET_WARNNING, "not supported now");
    return -1;
}

static int __connect(Inet_Udp_Socket *socket, char *host, char *service)
{
    dbg_str(NET_WARNNING, "not supported now");
    return -1;
}

static ssize_t __send(Inet_Udp_Socket *socket, const void *buf, size_t len, int flags)
{
    dbg_str(NET_WARNNING, "not supported now");
    return -1;
}

static ssize_t __recv(Inet_Udp_Socket *socket, void *buf, size_t len, int flags)
{
    dbg_str(NET_WARNNING, "not supported now");
    return -1;
}

static ssize_t 
__sendto(Inet_Udp_Socket *socket, const void *buf, size_t len, int flags, 
         char *host, char *service)
{
    dbg_str(NET_WARNNING, "not supported now");
    return -1;
}

static ssize_t 
__recvfrom(Inet_Udp_Socket *socket, void *buf, size_t len, int flags, 
           char *host, char *service)
{
    dbg_str(NET_WARNNING, "not supported now");
    return -1;
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
    dbg_str(NET_WARNNING, "not supported now");
    return -1;
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
    Init_End___Entry(14, Inet_Udp_Socket),
};
REGISTER_CLASS("Inet_Udp_Socket", inet_udp_socket_class_info);

void test_inet_udp_socket_send()
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();

    char *test_str = "hello world";

    socket = OBJECT_NEW(allocator, Inet_Udp_Socket, NULL);

    socket->connect(socket, "127.0.0.1", "11011");
    socket->send(socket, test_str, strlen(test_str), 0);

    //while(1) sleep(1);

    object_destroy(socket);
}
REGISTER_TEST_FUNC(test_inet_udp_socket_send);

void test_inet_udp_socket_recv()
{
    Socket *socket;
    char buf[1024] = {0};
    allocator_t *allocator = allocator_get_default_alloc();

    /*
     *dbg_str(NET_DETAIL, "run at here");
     */
    socket = OBJECT_NEW(allocator, Inet_Udp_Socket, NULL);

    socket->bind(socket, "127.0.0.1", "11011"); 
    //socket->setsockopt(socket, 0, 0, NULL);

    while(1) {
        socket->recv(socket, buf, 1024, 0);
        dbg_str(NET_SUC, "recv : %s", buf);
    }

    object_destroy(socket);
}
REGISTER_TEST_FUNC(test_inet_udp_socket_recv);
#endif