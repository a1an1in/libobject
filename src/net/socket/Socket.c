/**
 * @socket Socket.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-27
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
#include <fcntl.h> 
#include <errno.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/Event_Base.h>
#include <libobject/net/socket/Socket.h>

static int __get_sockoptval_size(int optname)
{
    int size = 0;

    switch(optname) {
        case SO_REUSEPORT:
        case SO_KEEPALIVE:
        case SO_REUSEADDR:
            size = sizeof(int);
            break;
        default:
            size = -1;
    }

    return size;
}

static int __construct(Socket *socket, char *init_str)
{
    allocator_t *allocator = socket->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(EV_DETAIL, "socket construct, socket addr:%p", socket);

    return 0;
}

static int __deconstrcut(Socket *socket)
{
    allocator_t *allocator = socket->obj.allocator;

    dbg_str(EV_DETAIL, "socket deconstruct, socket addr:%p", socket);

    return 0;
}

int __bind(Socket *socket, char *host, char *service)
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
        if ((ret = bind(socket->fd, addr->ai_addr, addr->ai_addrlen)) == 0)
            break;
    } while ((addr = addr->ai_next) != NULL);

    if (addr == NULL) {
        dbg_str(NET_WARNNING, "bind error for %s %s", host, service);
    }

    freeaddrinfo(addrsave);

    return ret;
}

static int __listen(Socket *socket, int backlog)
{
    int opt = 1;

    setsockopt(socket->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (listen(socket->fd, backlog) == -1) {
        perror("listen error");
        return -1;
    }

    return 0;
}


static int __connect(Socket *socket, char *host, char *service)
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
        if ((ret = connect(socket->fd, addr->ai_addr, addr->ai_addrlen)) == 0)
            break;
    } while ((addr = addr->ai_next) != NULL);

    if (addr == NULL) {
        dbg_str(NET_ERROR, "connect error for %s %s", host, service);
    }

    freeaddrinfo(addrsave);

    return ret;
}

static ssize_t __write(Socket *socket, const void *buf, size_t len)
{
    dbg_str(NET_DETAIL, "socket write");

    return write(socket->fd, buf, len);
}

static ssize_t __send(Socket *socket, const void *buf, size_t len, int flags)
{
    int ret;

    ret = send(socket->fd, buf, len, flags);

    if (ret <= 0) {
        dbg_str(NET_ERROR, "send error: %s", strerror(errno));
    }

    return ret;
}

static ssize_t 
__sendto(Socket *socket, const void *buf, size_t len,
         int flags, const struct sockaddr *dest_addr, 
         socklen_t addrlen)
{
    dbg_str(NET_DETAIL, "not supported now");
    return -1;
    /*
     *return sendto(socket->fd, buf, len, flags, dest_addr, addrlen);
     */
}

static ssize_t __sendmsg(Socket *socket, const struct msghdr *msg, int flags)
{
    dbg_str(NET_DETAIL, "not supported now");
    /*
     *return sendmsg(socket->fd, msg, flags);
     */
}

static ssize_t __read(Socket *socket, void *buf, size_t len)
{
    dbg_str(NET_DETAIL, "socket read, fd=%d", socket->fd);

    return read(socket->fd, buf, len);
}

static ssize_t __recv(Socket *socket, void *buf, size_t len, int flags)
{
    return recv(socket->fd, buf, len, flags);
}

static ssize_t __recvfrom(Socket *socket, void *buf, size_t len, int flags, 
                          struct sockaddr *src_addr, 
                          socklen_t *addrlen)
{
    dbg_str(NET_DETAIL, "not supported now");
    /*
     *return recvfrom(socket->fd, buf, len , flags, src_addr, addrlen);
     */
}

static ssize_t __recvmsg(Socket *socket, struct msghdr *msg, int flags)
{
    dbg_str(NET_DETAIL, "not supported now");
    /*
     *return recvmsg(socket->fd, msg, flags);
     */
}

static int __getsockopt(Socket *socket, int level, int optname, sockoptval *val)
{
    dbg_str(NET_DETAIL, "not supported now");
}

static int __setsockopt(Socket *socket, int level, int optname, sockoptval *val)
{
    int size;

    dbg_str(NET_DETAIL, "setsockopt level=%d, optname=%d", level, optname);

    size = __get_sockoptval_size(optname);
    if (size <= 0) {
        dbg_str(NET_WARNNING, "setsockopt level=%d, optname=%d, not supported now", 
                level, optname);
        return -1;
    }

    return setsockopt(socket->fd, level, optname, val, size);
}

static int __setnonblocking(Socket *socket)
{
    if (fcntl(socket->fd, F_SETFL, (fcntl(socket->fd, F_GETFD, 0) | O_NONBLOCK)) == -1) 
    {
        return -1;
    }                     

    return 0;
}

static class_info_entry_t socket_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Socket, construct, __construct),
    Init_Nfunc_Entry(2 , Socket, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Socket, set, NULL),
    Init_Vfunc_Entry(4 , Socket, get, NULL),
    Init_Vfunc_Entry(5 , Socket, bind, __bind),
    Init_Vfunc_Entry(6 , Socket, listen, __listen),
    Init_Vfunc_Entry(7 , Socket, accept, NULL),
    Init_Vfunc_Entry(8 , Socket, accept_fd, NULL),
    Init_Vfunc_Entry(9 , Socket, connect, __connect),
    Init_Vfunc_Entry(10, Socket, write, __write),
    Init_Vfunc_Entry(11, Socket, send, __send),
    Init_Vfunc_Entry(12, Socket, sendto, __sendto),
    Init_Vfunc_Entry(13, Socket, sendmsg, __sendmsg),
    Init_Vfunc_Entry(14, Socket, read, __read),
    Init_Vfunc_Entry(15, Socket, recv, __recv),
    Init_Vfunc_Entry(16, Socket, recvfrom, __recvfrom),
    Init_Vfunc_Entry(17, Socket, recvmsg, __recvmsg),
    Init_Vfunc_Entry(18, Socket, getsockopt, __getsockopt),
    Init_Vfunc_Entry(19, Socket, setsockopt, __setsockopt),
    Init_Vfunc_Entry(20, Socket, setnonblocking, __setnonblocking),
    Init_Point_Entry(21, Socket, local_host, NULL),
    Init_Point_Entry(22, Socket, local_service, NULL),
    Init_Point_Entry(23, Socket, remote_host, NULL),
    Init_Point_Entry(24, Socket, remote_service, NULL),
    Init_End___Entry(25, Socket),
};
REGISTER_CLASS("Socket", socket_class_info);

void test_obj_socket()
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(EV_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/Socket", CJSON_STRING, "name", "alan socket") ;  

    socket = OBJECT_NEW(allocator, Socket, c->buf);

    object_dump(socket, "Socket", buf, 2048);
    dbg_str(EV_DETAIL, "Socket dump: %s", buf);

    object_destroy(socket);
    cfg_destroy(c);
}
