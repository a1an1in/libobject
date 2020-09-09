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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/Event_Base.h>
#include <libobject/io/Socket.h>

static class_info_entry_t socket_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Socket, construct, NULL),
    Init_Nfunc_Entry(2 , Socket, deconstruct, NULL),
    Init_Vfunc_Entry(3 , Socket, set, NULL),
    Init_Vfunc_Entry(4 , Socket, get, NULL),
    Init_Vfunc_Entry(5 , Socket, bind, NULL),
    Init_Vfunc_Entry(6 , Socket, listen, NULL),
    Init_Vfunc_Entry(7 , Socket, accept, NULL),
    Init_Vfunc_Entry(8 , Socket, accept_fd, NULL),
    Init_Vfunc_Entry(9 , Socket, connect, NULL),
    Init_Vfunc_Entry(10, Socket, send, NULL),
    Init_Vfunc_Entry(11, Socket, recv, NULL),
    Init_Vfunc_Entry(12, Socket, sendto, NULL),
    Init_Vfunc_Entry(13, Socket, recvfrom, NULL),
    Init_Vfunc_Entry(14, Socket, getsockopt, NULL),
    Init_Vfunc_Entry(15, Socket, setsockopt, NULL),
    Init_Vfunc_Entry(16, Socket, setnonblocking, NULL),
    Init_Point_Entry(17, Socket, local_host, NULL),
    Init_Point_Entry(18, Socket, local_service, NULL),
    Init_Point_Entry(19, Socket, remote_host, NULL),
    Init_Point_Entry(20, Socket, remote_service, NULL),
    Init_End___Entry(21, Socket),
};
REGISTER_CLASS("Socket", socket_class_info);

