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
#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/event_base.h>
#include <libobject/net/socket/inet_tcp_socket.h>
#include <libobject/core/thread.h>

static int __construct(Inet_Tcp_Socket *sk, char *init_str)
{
    int skfd;

    dbg_str(NET_DETAIL, "socket construct, socket addr:%p", sk);

    if ((skfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("udp socket");
        return -1;
    }

    sk->parent.fd = skfd;

    return 0;
}

static int __deconstrcut(Inet_Tcp_Socket *socket)
{
    dbg_str(NET_DETAIL, "socket deconstruct, socket addr:%p", socket);

    close(socket->parent.fd);

    return 0;
}

static int __set(Inet_Tcp_Socket *socket, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        socket->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        socket->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        socket->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        socket->deconstruct = value;
    } 
    else if (strcmp(attrib, "bind") == 0) {
        socket->bind = value;
        dbg_str(DBG_DETAIL, "set bind, addr:%p", socket->bind);
    } else if (strcmp(attrib, "listen") == 0) {
        socket->listen = value;
    } else if (strcmp(attrib, "accept") == 0) {
        socket->accept = value;
    } else if (strcmp(attrib, "accept_fd") == 0) {
        socket->accept_fd = value;
    } else if (strcmp(attrib, "connect") == 0) {
        socket->connect = value;
    } else if (strcmp(attrib, "write") == 0) {
        socket->write = value;
    } else if (strcmp(attrib, "sendto") == 0) {
        socket->sendto = value;
    } else if (strcmp(attrib, "sendmsg") == 0) {
        socket->sendmsg = value;
    } else if (strcmp(attrib, "read") == 0) {
        socket->read = value;
    } else if (strcmp(attrib, "recv") == 0) {
        socket->recv = value;
    } else if (strcmp(attrib, "recvfrom") == 0) {
        socket->recvfrom = value;
    } else if (strcmp(attrib, "recvmsg") == 0) {
        socket->recvmsg = value;
    } 
    else {
        dbg_str(NET_DETAIL, "socket set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Inet_Tcp_Socket *socket, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(NET_WARNNING, "socket get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
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
        ret = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);
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

static class_info_entry_t inet_tcp_socket_class_info[] = {
    Init_Obj___Entry(0 , Socket, parent),
    Init_Nfunc_Entry(1 , Inet_Tcp_Socket, construct, __construct),
    Init_Nfunc_Entry(2 , Inet_Tcp_Socket, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Inet_Tcp_Socket, set, NULL),
    Init_Vfunc_Entry(4 , Inet_Tcp_Socket, get, NULL),
    Init_Vfunc_Entry(5 , Inet_Tcp_Socket, bind, NULL),
    Init_Vfunc_Entry(6 , Inet_Tcp_Socket, listen, NULL),
    Init_Vfunc_Entry(7 , Inet_Tcp_Socket, accept, __accept),
    Init_Vfunc_Entry(8 , Inet_Tcp_Socket, accept_fd, __accept_fd),
    Init_Vfunc_Entry(9 , Inet_Tcp_Socket, connect, NULL),
    Init_Vfunc_Entry(10, Inet_Tcp_Socket, write, NULL),
    Init_Vfunc_Entry(11, Inet_Tcp_Socket, sendto, NULL),
    Init_Vfunc_Entry(12, Inet_Tcp_Socket, sendmsg, NULL),
    Init_Vfunc_Entry(13, Inet_Tcp_Socket, read, NULL),
    Init_Vfunc_Entry(14, Inet_Tcp_Socket, recv, NULL),
    Init_Vfunc_Entry(15, Inet_Tcp_Socket, recvfrom, NULL),
    Init_Vfunc_Entry(16, Inet_Tcp_Socket, recvmsg, NULL),
    Init_End___Entry(17),
};
REGISTER_CLASS("Inet_Tcp_Socket", inet_tcp_socket_class_info);

void test_inet_tcp_socket_send()
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();

    char *test_str = "hello world";

    socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);

    socket->connect(socket, "127.0.0.1", "11011");
    socket->write(socket, test_str, strlen(test_str));

    while(1) sleep(1);

    object_destroy(socket);
}

void test_inet_tcp_socket_recv()
{
    Socket *socket, *new;
    char buf[1024] = {0};
    allocator_t *allocator = allocator_get_default_alloc();

    /*
     *dbg_str(NET_DETAIL, "run at here");
     */
    socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);

    dbg_str(DBG_DETAIL, "sizeof socket=%d", sizeof(Socket));
    socket->bind(socket, "127.0.0.1", "11011"); 
    socket->listen(socket, 1024);
    new = socket->accept(socket, NULL, NULL);

    new->read(new, buf, 1024);
    dbg_str(NET_SUC, "recv : %s", buf);

    sleep(10);

    object_destroy(new);
    object_destroy(socket);
}

