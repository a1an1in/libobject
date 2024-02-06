/**
 * @file Inet_Udp_V6_Client.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2024-02-05
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
#include <libobject/concurrent/net/Inet_Udp_V6_Client.h>

static int __construct(Inet_Udp_V6_Client *client, char *init_str)
{
    allocator_t *allocator = client->parent.obj.allocator;

    dbg_str(NET_DETAIL, "Inet_Udp_V6_Client construct, client addr:%p", client);
    client->parent.socket = object_new(allocator, "Inet_Udp_V6_Socket", NULL);
    if (client->parent.socket == NULL) {
        dbg_str(NET_ERROR, "OBJECT_NEW Inet_Udp_V6_Socket");
        return -1;
    }

    client->parent.worker = object_new(allocator, "Worker", NULL);
    if (client->parent.worker == NULL) {
        dbg_str(NET_ERROR, "OBJECT_NEW Worker");
        return -1;
    }

    return 0;
}

static int __deconstrcut(Inet_Udp_V6_Client *client)
{
    dbg_str(NET_DETAIL, "Inet_Udp_V6_Client deconstruct, client addr:%p", client);
    object_destroy(client->parent.socket);
    object_destroy(client->parent.worker);

    return 0;
}

static class_info_entry_t inet_udp_v6_client_class_info[] = {
    Init_Obj___Entry(0 , Client, parent),
    Init_Nfunc_Entry(1 , Inet_Udp_V6_Client, construct, __construct),
    Init_Nfunc_Entry(2 , Inet_Udp_V6_Client, deconstruct, __deconstrcut),
    Init_End___Entry(3 , Inet_Udp_V6_Client),
};
REGISTER_CLASS("Inet_Udp_V6_Client", inet_udp_v6_client_class_info);

