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
#include <errno.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/Event_Base.h>
#include <libobject/core/Thread.h>
#include "Inet_Tcp_Sub_Socket.h"

static int __construct(Inet_Tcp_Sub_Socket *sk, char *init_str)
{
    return 0;
}

static int __deconstrcut(Inet_Tcp_Sub_Socket *socket)
{
    if (socket->parent.fd)
        close(socket->parent.fd);

    return 0;
}

static Socket * __accept(Inet_Tcp_Sub_Socket *socket, 
                         char *remote_host, char *remote_service)
{
    dbg_str(DBG_WARNNING, "Inet_Tcp_Sub_Socket not support accept operation");
    return NULL;
}

static int __accept_fd(Inet_Tcp_Sub_Socket *socket, 
                       char *remote_host, char *remote_service)
{
    dbg_str(DBG_WARNNING, "Inet_Tcp_Sub_Socket not support accept operation");
    return -1;
}

static class_info_entry_t inet_tcp_sub_socket_class_info[] = {
    Init_Obj___Entry(0 , Socket, parent),
    Init_Nfunc_Entry(1 , Inet_Tcp_Sub_Socket, construct, __construct),
    Init_Nfunc_Entry(2 , Inet_Tcp_Sub_Socket, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Inet_Tcp_Sub_Socket, set, NULL),
    Init_Vfunc_Entry(4 , Inet_Tcp_Sub_Socket, get, NULL),
    Init_Vfunc_Entry(5 , Inet_Tcp_Sub_Socket, bind, NULL),
    Init_Vfunc_Entry(6 , Inet_Tcp_Sub_Socket, listen, NULL),
    Init_Vfunc_Entry(7 , Inet_Tcp_Sub_Socket, accept, __accept),
    Init_Vfunc_Entry(8 , Inet_Tcp_Sub_Socket, accept_fd, __accept_fd),
    Init_Vfunc_Entry(9 , Inet_Tcp_Sub_Socket, connect, NULL),
    Init_Vfunc_Entry(10, Inet_Tcp_Sub_Socket, write, NULL),
    Init_Vfunc_Entry(11, Inet_Tcp_Sub_Socket, sendto, NULL),
    Init_Vfunc_Entry(12, Inet_Tcp_Sub_Socket, sendmsg, NULL),
    Init_Vfunc_Entry(13, Inet_Tcp_Sub_Socket, read, NULL),
    Init_Vfunc_Entry(14, Inet_Tcp_Sub_Socket, recv, NULL),
    Init_Vfunc_Entry(15, Inet_Tcp_Sub_Socket, recvfrom, NULL),
    Init_Vfunc_Entry(16, Inet_Tcp_Sub_Socket, recvmsg, NULL),
    Init_End___Entry(17, Inet_Tcp_Sub_Socket),
};
REGISTER_CLASS("Inet_Tcp_Sub_Socket", inet_tcp_sub_socket_class_info);

