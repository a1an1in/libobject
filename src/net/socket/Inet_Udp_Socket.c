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
#include <libobject/core/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/Event_Base.h>
#include <libobject/net/socket/Inet_Udp_Socket.h>
#include <libobject/core/Thread.h>

static int __construct(Inet_Udp_Socket *sk, char *init_str)
{
    int skfd;


    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("udp socket");
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

static int __set(Inet_Udp_Socket *socket, char *attrib, void *value)
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

static void *__get(Inet_Udp_Socket *socket, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(NET_WARNNING, "socket get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t inet_udp_socket_class_info[] = {
    Init_Obj___Entry(0 , Socket, parent),
    Init_Nfunc_Entry(1 , Inet_Udp_Socket, construct, __construct),
    Init_Nfunc_Entry(2 , Inet_Udp_Socket, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Inet_Udp_Socket, set, NULL),
    Init_Vfunc_Entry(4 , Inet_Udp_Socket, get, NULL),
    Init_Vfunc_Entry(5 , Inet_Udp_Socket, bind, NULL),
    Init_Vfunc_Entry(6 , Inet_Udp_Socket, connect, NULL),
    Init_Vfunc_Entry(7 , Inet_Udp_Socket, write, NULL),
    Init_Vfunc_Entry(8 , Inet_Udp_Socket, sendto, NULL),
    Init_Vfunc_Entry(9 , Inet_Udp_Socket, sendmsg, NULL),
    Init_Vfunc_Entry(10, Inet_Udp_Socket, read, NULL),
    Init_Vfunc_Entry(11, Inet_Udp_Socket, recv, NULL),
    Init_Vfunc_Entry(12, Inet_Udp_Socket, recvfrom, NULL),
    Init_Vfunc_Entry(13, Inet_Udp_Socket, recvmsg, NULL),
    Init_End___Entry(14, Inet_Udp_Socket),
};
REGISTER_CLASS("Inet_Udp_Socket", inet_udp_socket_class_info);

#if 0
void test_obj_udp_socket()
{
    Inet_Udp_Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(NET_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/Inet_Udp_Socket", CJSON_STRING, "name", "alan socket") ;  
    cfg_config(c, "/Inet_Udp_Socket", CJSON_STRING, "local_host", "192.168.1.122") ;  
    cfg_config(c, "/Inet_Udp_Socket", CJSON_STRING, "local_service", "11011") ;  
    cfg_config(c, "/Inet_Udp_Socket", CJSON_STRING, "remote_host", "192.168.1.122") ;  
    cfg_config(c, "/Inet_Udp_Socket", CJSON_STRING, "remote_service", "11022") ;  

    socket = OBJECT_NEW(allocator, Inet_Udp_Socket, c->buf);

    object_dump(socket, "Inet_Udp_Socket", buf, 2048);
    dbg_str(NET_DETAIL, "Inet_Udp_Socket dump: %s", buf);

    socket->write(socket, "hello world", 5);
    socket->read(socket, NULL, 0);

    object_destroy(socket);
    cfg_destroy(c);
}
#endif

void test_inet_udp_socket_send()
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();

    char *test_str = "hello world";

    socket = OBJECT_NEW(allocator, Inet_Udp_Socket, NULL);

    socket->connect(socket, "127.0.0.1", "11011");
    socket->write(socket, test_str, strlen(test_str));

    while(1) sleep(1);

    object_destroy(socket);
}

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
    socket->setsockopt(socket, 0, 0, NULL);

    while(1) {
        socket->read(socket, buf, 1024);
        dbg_str(NET_SUC, "recv : %s", buf);
    }

    object_destroy(socket);
}

