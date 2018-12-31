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
#include <libobject/net/socket/unix_udp_socket.h>
#include <libobject/core/thread.h>
#include <sys/un.h>

static int __construct(Unix_Udp_Socket *sk, char *init_str)
{
    int skfd;

    if ((skfd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("udp socket");
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

static int __set(Unix_Udp_Socket *socket, char *attrib, void *value)
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

static void *__get(Unix_Udp_Socket *socket, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(NET_WARNNING, "socket get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
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
         /*
          *perror("fail to bind");
          */
         dbg_str(DBG_ERROR,"fail to bind, error_no=%d", ret);
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
         dbg_str(DBG_ERROR,"unix udp, fail to connect, error_no=%d", ret);
         ret = -1;
     }                         

     return ret;
}

static class_info_entry_t inet_udp_socket_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Socket", "parent", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "bind", __bind, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "connect", __connect, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "write", NULL, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "sendto", NULL, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "sendmsg", NULL, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_IFUNC_POINTER, "", "read", NULL, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_IFUNC_POINTER, "", "recv", NULL, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_IFUNC_POINTER, "", "recvfrom", NULL, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_IFUNC_POINTER, "", "recvmsg", NULL, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Unix_Udp_Socket", inet_udp_socket_class_info);

#define UNIX_CLIENT_PATH "/tmp/unix_client_01"
#define UNIX_SERVER_PATH "/tmp/unix_server_01"

void test_unix_udp_socket_send()
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();

    char *test_str = "hello world";

    socket = OBJECT_NEW(allocator, Unix_Udp_Socket, NULL);

    socket->connect(socket, UNIX_SERVER_PATH, NULL);
    socket->write(socket, test_str, strlen(test_str));

    while(1) sleep(1);

    object_destroy(socket);
}

void test_unix_udp_socket_recv()
{
    Socket *socket;
    char buf[1024] = {0};
    allocator_t *allocator = allocator_get_default_alloc();

    socket = OBJECT_NEW(allocator, Unix_Udp_Socket, NULL);

    socket->bind(socket, UNIX_SERVER_PATH, NULL); 
    /*
     *socket->setsockopt(socket, 0, 0, NULL);
     */

    while(1) {
        socket->read(socket, buf, 1024);
        dbg_str(NET_SUC, "recv : %s", buf);
    }

    object_destroy(socket);
}

