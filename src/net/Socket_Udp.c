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
#include <libobject/core/thread.h>

static int __construct(Udp_Socket *sk,char *init_str)
{
    int skfd;

    dbg_str(NET_DETAIL,"socket construct, socket addr:%p",sk);

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("udp socket");
        return -1;
    }

    sk->parent.fd = skfd;

    return 0;
}

static int __deconstrcut(Udp_Socket *socket)
{
    dbg_str(NET_DETAIL,"socket deconstruct,socket addr:%p",socket);

    close(socket->parent.fd);

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
        dbg_str(DBG_DETAIL,"set bind, addr:%p", socket->bind);
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
    /*
     *else if (strcmp(attrib, "local_host") == 0) {
     *    strncpy(socket->local_host, value, strlen(value));
     *} else if (strcmp(attrib, "local_service") == 0) {
     *    strncpy(socket->local_service, value, strlen(value));
     *} else if (strcmp(attrib, "remote_host") == 0) {
     *    strncpy(socket->remote_host, value, strlen(value));
     *} else if (strcmp(attrib, "remote_service") == 0) {
     *    strncpy(socket->remote_service, value, strlen(value));
     *}
     */
    else {
        dbg_str(NET_DETAIL,"socket set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Udp_Socket *socket, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    /*
     *} else if (strcmp(attrib, "local_host") == 0) {
     *    return socket->local_host;
     *} else if (strcmp(attrib, "local_service") == 0) {
     *    return socket->local_service;
     *} else if (strcmp(attrib, "remote_host") == 0) {
     *    return socket->remote_host;
     *} else if (strcmp(attrib, "remote_service") == 0) {
     *    return socket->remote_service;
     */
    } else {
        dbg_str(NET_WARNNING,"socket get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

#if 0
int __bind(Udp_Socket *socket, char *host, char *service)
{
    struct addrinfo  *addr, *addrsave, hint;
    int skfd, ret;
    char *h, *s;

    bzero(&hint, sizeof(hint));
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_DGRAM;

    if (host == NULL && service == NULL) {
        h = socket->local_host;
        s = socket->local_service;
    } else {
        h = host;
        s = service;
    }

    if ((ret = getaddrinfo(h, s, &hint, &addr)) != 0){
        dbg_str(DBG_ERROR,"getaddrinfo error: %s", gai_strerror(ret));
        return -1;
    }
    addrsave = addr;

    if(addr != NULL){
        dbg_str(NET_DETAIL,"ai_family=%d type=%d",addr->ai_family,addr->ai_socktype);
    }else{
        dbg_str(DBG_ERROR,"getaddrinfo err");
        return -1;
    }                      

    do {
        if ((ret = bind(socket->fd, addr->ai_addr, addr->ai_addrlen)) == 0)
            break;
    } while ((addr = addr->ai_next) != NULL);

    if (addr == NULL) {
        dbg_str(NET_WARNNING,"bind error for %s %s", host, service);
    }

    freeaddrinfo(addrsave);

    return ret;
}

int __connect(Udp_Socket *socket, char *host, char *service)
{
    struct addrinfo  *addr, *addrsave, hint;
    int skfd, ret;
    char *h, *s;

    bzero(&hint, sizeof(hint));
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_DGRAM;

    if (host == NULL && service == NULL) {
        h = socket->remote_host;
        s = socket->remote_service;
    } else {
        h = host;
        s = service;
    }

    if ((ret = getaddrinfo(h, s, &hint, &addr)) != 0){
        dbg_str(DBG_ERROR,"getaddrinfo error: %s", gai_strerror(ret));
        return -1;
    }
    addrsave = addr;

    if(addr != NULL){
        dbg_str(NET_DETAIL,"ai_family=%d type=%d",addr->ai_family,addr->ai_socktype);
    }else{
        dbg_str(DBG_ERROR,"getaddrinfo err");
        return -1;
    }                      

    do {
        if ((ret = connect(socket->fd, addr->ai_addr, addr->ai_addrlen)) == 0)
            break;
    } while ((addr = addr->ai_next) != NULL);

    if (addr == NULL) {
        dbg_str(NET_WARNNING,"connect error for %s %s", host, service);
    }

    freeaddrinfo(addrsave);

    return ret;
}

ssize_t __write(Udp_Socket *socket, const void *buf, size_t len)
{
    dbg_str(NET_DETAIL, "udp socket write");

    return write(socket->fd, buf, len);
}

ssize_t __send(Udp_Socket *socket, const void *buf, size_t len, int flags)
{
    return send(socket->fd, buf, len, flags);
}

ssize_t __sendto(Udp_Socket *socket, const void *buf, size_t len, int flags,
                 const struct sockaddr *dest_addr,
                 socklen_t addrlen)
{
    dbg_str(NET_DETAIL, "udp socket send to, fd=%d, buf %s",socket->fd, buf);
    return sendto(socket->fd, buf, len, flags, dest_addr, addrlen);
}

ssize_t __sendmsg(Udp_Socket *socket, const struct msghdr *msg, int flags)
{
    return sendmsg(socket->fd, msg, flags);
}

ssize_t __read(Udp_Socket *socket, void *buf, size_t len)
{
    dbg_str(NET_DETAIL, "udp socket read, fd=%d", socket->fd);

    return read(socket->fd, buf, len);
}

ssize_t __recv(Udp_Socket *socket, void *buf, size_t len, int flags)
{
    return recv(socket->fd, buf, len, flags);
}

ssize_t __recvfrom(Udp_Socket *socket, void *buf, size_t len, int flags,
                   struct sockaddr *src_addr, 
                   socklen_t *addrlen)
{
    return recvfrom(socket->fd, buf, len ,flags, src_addr, addrlen);
}

ssize_t __recvmsg(Udp_Socket *socket, struct msghdr *msg, int flags)
{
    return recvmsg(socket->fd, msg, flags);
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
    [14] = {ENTRY_TYPE_STRING,"","local_host",NULL,sizeof(void *)},
    [15] = {ENTRY_TYPE_STRING,"","local_service",NULL,sizeof(void *)},
    [16] = {ENTRY_TYPE_STRING,"","remote_host",NULL,sizeof(void *)},
    [17] = {ENTRY_TYPE_STRING,"","remote_service",NULL,sizeof(void *)},
    [18] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Udp_Socket",udp_socket_class_info);
#endif
static class_info_entry_t udp_socket_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Socket","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_IFUNC_POINTER,"","bind", NULL,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_IFUNC_POINTER,"","connect",NULL,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_IFUNC_POINTER,"","write",NULL,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_IFUNC_POINTER,"","sendto",NULL,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_IFUNC_POINTER,"","sendmsg",NULL,sizeof(void *)},
    [10] = {ENTRY_TYPE_IFUNC_POINTER,"","read",NULL,sizeof(void *)},
    [11] = {ENTRY_TYPE_IFUNC_POINTER,"","recv",NULL,sizeof(void *)},
    [12] = {ENTRY_TYPE_IFUNC_POINTER,"","recvfrom",NULL,sizeof(void *)},
    [13] = {ENTRY_TYPE_IFUNC_POINTER,"","recvmsg",NULL,sizeof(void *)},
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
    cfg_config(c, "/Udp_Socket", CJSON_STRING, "local_host", "192.168.1.122") ;  
    cfg_config(c, "/Udp_Socket", CJSON_STRING, "local_service", "11011") ;  
    cfg_config(c, "/Udp_Socket", CJSON_STRING, "remote_host", "192.168.1.122") ;  
    cfg_config(c, "/Udp_Socket", CJSON_STRING, "remote_service", "11022") ;  

    socket = OBJECT_NEW(allocator, Udp_Socket,c->buf);

    object_dump(socket, "Udp_Socket", buf, 2048);
    dbg_str(NET_DETAIL,"Udp_Socket dump: %s",buf);

    socket->write(socket, "hello world", 5);
    socket->read(socket, NULL, 0);

    object_destroy(socket);
    cfg_destroy(c);
}

void test_udp_socket_send()
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();

    char *test_str = "hello world";

    socket = OBJECT_NEW(allocator, Udp_Socket, NULL);

    socket->connect(socket, "127.0.0.1", "11011");
    socket->write(socket, test_str, strlen(test_str));

    while(1) sleep(1);

    object_destroy(socket);
}

void test_udp_socket_recv()
{
    Socket *socket;
    char buf[1024] = {0};
    allocator_t *allocator = allocator_get_default_alloc();

    /*
     *dbg_str(NET_DETAIL,"run at here");
     */
    socket = OBJECT_NEW(allocator, Udp_Socket, NULL);

    socket->bind(socket, "127.0.0.1", "11011"); 

    while(1) {
        socket->read(socket, buf, 1024);
        dbg_str(NET_SUC,"recv : %s",buf);
    }

    object_destroy(socket);
}

