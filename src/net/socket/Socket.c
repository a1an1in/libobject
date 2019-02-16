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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/event_base.h>
#include <libobject/net/socket/socket.h>

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

    socket->local_host     = allocator_mem_alloc(allocator, DEFAULT_MAX_IP_STR_LEN);
    socket->local_service  = allocator_mem_alloc(allocator, DEFAULT_MAX_PORT_STR_LEN);
    socket->remote_host    = allocator_mem_alloc(allocator, DEFAULT_MAX_IP_STR_LEN);
    socket->remote_service = allocator_mem_alloc(allocator, DEFAULT_MAX_PORT_STR_LEN);
    
    socket->fd = -1;

    return 0;
}

static int __deconstrcut(Socket *socket)
{
    allocator_t *allocator = socket->obj.allocator;

    dbg_str(EV_DETAIL, "socket deconstruct, socket addr:%p", socket);
    allocator_mem_free(allocator, socket->local_host);
    allocator_mem_free(allocator, socket->local_service);
    allocator_mem_free(allocator, socket->remote_host);
    allocator_mem_free(allocator, socket->remote_service);

    return 0;
}

static int __set(Socket *socket, char *attrib, void *value)
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
    } else if (strcmp(attrib, "send") == 0) {
        socket->send = value;
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
    } else if (strcmp(attrib, "getsockopt") == 0) {
        socket->getsockopt = value;
    } else if (strcmp(attrib, "setsockopt") == 0) {
        socket->setsockopt = value;
    } else if (strcmp(attrib, "setnonblocking") == 0) {
        socket->setnonblocking = value;
    } else if (strcmp(attrib, "close") == 0) {
        socket->close = value;
    } else if (strcmp(attrib, "setblock") == 0) {
        socket->setblock = value;
    } else if (strcmp(attrib, "shutdown") == 0) {
        socket->shutdown = value;
    } else if (strcmp(attrib, "setnoclosewait") == 0) {
        socket->setnoclosewait = value;
    } else if (strcmp(attrib, "setclosewait") == 0) {
        socket->setclosewait = value;
    } else if (strcmp(attrib, "setclosewaitdefault") == 0) {
        socket->setclosewaitdefault = value;
    } else if (strcmp(attrib, "get_timeout") == 0) {
        socket->get_timeout = value;
    } else if (strcmp(attrib, "set_timeout") == 0) {
        socket->set_timeout = value;
    } else if (strcmp(attrib, "get_socketfd") == 0) {
        socket->get_socketfd = value;
    } 
    else if (strcmp(attrib, "local_host") == 0) {
        strncpy(socket->local_host, value, strlen(value));
    } else if (strcmp(attrib, "local_service") == 0) {
        strncpy(socket->local_service, value, strlen(value));
    } else if (strcmp(attrib, "remote_host") == 0) {
        strncpy(socket->remote_host, value, strlen(value));
    } else if (strcmp(attrib, "remote_service") == 0) {
        strncpy(socket->remote_service, value, strlen(value));
    }
    else {
        dbg_str(EV_DETAIL, "socket set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Socket *socket, char *attrib)
{
    if (strcmp(attrib, "local_host") == 0) {
        return socket->local_host;
    } else if (strcmp(attrib, "local_service") == 0) {
        return socket->local_service;
    } else if (strcmp(attrib, "remote_host") == 0) {
        return socket->remote_host;
    } else if (strcmp(attrib, "remote_service") == 0) {
        return socket->remote_service;
    }
    else {
        dbg_str(EV_WARNNING, "socket get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static int __bind(Socket *socket, char *host, char *service)
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

    socket->setsockopt(socket,SOL_SOCKET,SO_REUSEADDR, (const void *)addr->ai_addr);

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
        dbg_str(NET_WARNNING, "connect error for %s %s", host, service);
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
    return send(socket->fd, buf, len, flags);
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

static int __close(Socket *socket)
{
    return close(socket->fd);
}

static  void __setblock(Socket *socket,int bBlock)
{
    int val = 0;

    if ((val = fcntl(socket->fd, F_GETFL, 0)) == -1) {   
        dbg_str(DBG_ERROR,"Socket fcntl getfl failed!");
        return ;
    }
    if(!bBlock) {
        val |= O_NONBLOCK;
    }
    else {
        val &= ~O_NONBLOCK;
    }

    if (fcntl(socket->fd, F_SETFL, val) == -1) {
        dbg_str(DBG_ERROR,"Socket fcntl setfl failed!");
        return;
    }
}

static int __shutdown(Socket *socket,int ihow)
{
    return shutdown(socket->fd,ihow);
}

static void __setnoclosewait(Socket *socket)
{
    struct linger stLinger;
    stLinger.l_onoff = 1;       
    stLinger.l_linger = 0;                  

    if(socket->setsockopt(socket,SOL_SOCKET,SO_LINGER, (const void *)&stLinger) == -1) {
        dbg_str(DBG_ERROR,"Socket setnoclosewait  failed!");
    }
}

static void __setclosewait(Socket *socket,int delaytime)
{
    struct linger  stLinger;
    stLinger.l_onoff = 1;  
    stLinger.l_linger = delaytime; 

    if(socket->setsockopt(socket,SOL_SOCKET,SO_LINGER, (const void *)&stLinger) == -1) {
        dbg_str(DBG_ERROR,"Socket setclosewait  failed!");
    }
}

static void __setclosewaitdefault(Socket *socket)
{
    struct linger stLinger;
    stLinger.l_onoff  = 0;     
    stLinger.l_linger = 0;

    if(socket->setsockopt(socket,SOL_SOCKET,SO_LINGER, (const void *)&stLinger) == -1) {
        dbg_str(DBG_ERROR,"Socket setclosewaitdefault  failed!");
    }
}

static class_info_entry_t socket_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "bind", __bind, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "listen", __listen, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "accept", NULL, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "accept_fd", NULL, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "connect", __connect, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "write", __write, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "send", __send, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_VFUNC_POINTER, "", "sendto", __sendto, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_VFUNC_POINTER, "", "sendmsg", __sendmsg, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_VFUNC_POINTER, "", "read", __read, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_VFUNC_POINTER, "", "recv", __recv, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_VFUNC_POINTER, "", "recvfrom", __recvfrom, sizeof(void *)}, 
    [17] = {ENTRY_TYPE_VFUNC_POINTER, "", "recvmsg", __recvmsg, sizeof(void *)}, 
    [18] = {ENTRY_TYPE_VFUNC_POINTER, "", "getsockopt", __getsockopt, sizeof(void *)}, 
    [19] = {ENTRY_TYPE_VFUNC_POINTER, "", "setsockopt", __setsockopt, sizeof(void *)}, 
    [20] = {ENTRY_TYPE_VFUNC_POINTER, "", "setnonblocking", __setnonblocking, sizeof(void *)}, 
    [21] = {ENTRY_TYPE_VFUNC_POINTER, "", "close", __close, sizeof(void *)}, 
    [22] = {ENTRY_TYPE_VFUNC_POINTER, "", "shutdown", __shutdown, sizeof(void *)}, 
    [23] = {ENTRY_TYPE_VFUNC_POINTER, "", "setclosewait", __setclosewait, sizeof(void *)}, 
    [24] = {ENTRY_TYPE_VFUNC_POINTER, "", "setnoclosewait", __setnoclosewait, sizeof(void *)}, 
    [25] = {ENTRY_TYPE_VFUNC_POINTER, "", "setclosewaitdefault", __setclosewaitdefault, sizeof(void *)}, 
    [26] = {ENTRY_TYPE_VFUNC_POINTER, "", "setblock", __setblock, sizeof(void *)}, 
    [27] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_timeout", NULL, sizeof(void *)}, 
    [28] = {ENTRY_TYPE_VFUNC_POINTER, "", "set_timeout", NULL, sizeof(void *)}, 
    [29] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_socketfd", NULL, sizeof(void *)}, 
    [30] = {ENTRY_TYPE_STRING, "", "local_host", NULL, sizeof(void *)}, 
    [31] = {ENTRY_TYPE_STRING, "", "local_service", NULL, sizeof(void *)}, 
    [32] = {ENTRY_TYPE_STRING, "", "remote_host", NULL, sizeof(void *)}, 
    [33] = {ENTRY_TYPE_STRING, "", "remote_service", NULL, sizeof(void *)}, 
    [34] = {ENTRY_TYPE_END}, 
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
