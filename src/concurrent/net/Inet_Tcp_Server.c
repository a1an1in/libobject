/**
 * @file Inet_Tcp_Server.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-10-24
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
#include <libobject/core/utils/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/net/Inet_Tcp_Server.h>

static int __construct(Inet_Tcp_Server *server, char *init_str)
{
    allocator_t *allocator = server->parent.obj.allocator;
    int opt = 1;

    dbg_str(NET_DETAIL, "Inet_Tcp_Server construct, server addr:%p", server);
    server->parent.socket = object_new(allocator, "Inet_Tcp_Socket", NULL);
    if (server->parent.socket == NULL) {
        dbg_str(NET_ERROR, "new Inet_Udp_Socket error");
        return -1;
    }

    server->parent.worker = object_new(allocator, "Worker", NULL);
    if (server->parent.worker == NULL) {
        dbg_str(NET_ERROR, "new Worker error");
        return -1;
    }

    return 0;
}

static int __deconstrcut(Inet_Tcp_Server *server)
{
    dbg_str(NET_DETAIL, "server deconstruct, server addr:%p", server);
    object_destroy(server->parent.socket);
    object_destroy(server->parent.worker);

    return 0;
}

static class_info_entry_t concurent_class_info[] = {
    Init_Obj___Entry(0 , Server, parent),
    Init_Nfunc_Entry(1 , Inet_Tcp_Server, construct, __construct),
    Init_Nfunc_Entry(2 , Inet_Tcp_Server, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Inet_Tcp_Server, set, NULL),
    Init_Vfunc_Entry(4 , Inet_Tcp_Server, get, NULL),
    Init_Vfunc_Entry(5 , Inet_Tcp_Server, bind, NULL),
    Init_Vfunc_Entry(6 , Inet_Tcp_Server, trustee, NULL),
    Init_End___Entry(7 , Inet_Tcp_Server),
};
REGISTER_CLASS("Inet_Tcp_Server", concurent_class_info);

static void test_work_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    dbg_str(NET_SUC, "%s", t->buf);
}

void test_obj_inet_tcp_server()
{
    Inet_Tcp_Server *server;
    allocator_t *allocator = allocator_get_default_instance();

    /*
     *sleep(1);
     */
    server = object_new(allocator, "Inet_Tcp_Server", NULL);
    server->bind(server, "127.0.0.1", "11011"); 
    server->trustee(server, test_work_callback, NULL);

#if (defined(WINDOWS_USER_MODE))
    sleep(1000);
#else
    pause();
#endif

    object_destroy(server);
}
