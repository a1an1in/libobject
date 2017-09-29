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
#include <libobject/utils/dbg/debug.h>
#include <libobject/event/event_base.h>
#include <libobject/utils/config/config.h>
#include <libobject/utils/timeval/timeval.h>
#include <libobject/net/socket_udp.h>

static int __construct(Udp_Socket *socket,char *init_str)
{
    configurator_t * c;
    char buf[2048];

    dbg_str(NET_DETAIL,"socket construct, socket addr:%p",socket);


    return 0;
}

static int __deconstrcut(Udp_Socket *socket)
{
    dbg_str(NET_DETAIL,"socket deconstruct,socket addr:%p",socket);

    return 0;
}

static int __set(Udp_Socket *socket, char *attrib, void *value)
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
        dbg_str(NET_DETAIL,"socket set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Udp_Socket *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(NET_WARNNING,"socket get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

int __bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{

}

int __connect(Udp_Socket *socket, const struct sockaddr *addr, socklen_t addrlen)
{
}

ssize_t __write(Udp_Socket *socket, const void *buf, size_t len)
{
    dbg_str(NET_DETAIL, "udp socket write");
}

ssize_t __send(Udp_Socket *socket, const void *buf, size_t len, int flags)
{
}

ssize_t __sendto(Udp_Socket *socket, const void *buf, size_t len, int flags,
                 const struct sockaddr *dest_addr,
                 socklen_t addrlen)
{
}

ssize_t __sendmsg(Udp_Socket *socket, const struct msghdr *msg, int flags)
{
}

ssize_t __read(Udp_Socket *socket, const void *buf, size_t len)
{
}

ssize_t __recv(Udp_Socket *socket, void *buf, size_t len, int flags)
{
}

ssize_t __recvfrom(Udp_Socket *socket, void *buf, size_t len, int flags,
                   struct sockaddr *src_addr, 
                   socklen_t *addrlen)
{
}

ssize_t __recvmsg(Udp_Socket *socket, struct msghdr *msg, int flags)
{
}

static class_info_entry_t udp_socket_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Socket","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","bind",__bind,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","connect",__connect,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","write",__write,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","sendto",__sendto,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","sendmsg",__sendmsg,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","read",__read,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","recv",__recv,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","recvfrom",__recvfrom,sizeof(void *)},
    [13] = {ENTRY_TYPE_VFUNC_POINTER,"","recvmsg",__recvmsg,sizeof(void *)},
    [14] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Udp_Socket",udp_socket_class_info);

void test_obj_udp_socket()
{
    Udp_Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(NET_SUC, "configurator_t addr:%p",c);
    cfg_config(c, "/Udp_Socket", CJSON_STRING, "name", "alan socket") ;  

    socket = OBJECT_NEW(allocator, Udp_Socket,c->buf);

    object_dump(socket, "Udp_Socket", buf, 2048);
    dbg_str(NET_DETAIL,"Udp_Socket dump: %s",buf);

    socket->write(socket, "hello world", 5);

    object_destroy(socket);
    cfg_destroy(c);
}


