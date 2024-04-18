/**
 * @file Client.c
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
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/net/Inet_Udp_Client.h>
#include <libobject/concurrent/net/Inet_Tcp_Client.h>
#include <libobject/concurrent/net/Inet_Tcp_Server.h>
#include <libobject/concurrent/net/Inet_Udp_V6_Client.h>
#include <libobject/concurrent/net/api.h>

void *client(allocator_t *allocator, char *type,
             char *host, char *service)
{
    Client *client = NULL;

    //there may add id_str check....
    dbg_str(DBG_VIP,"client, type:%s, host:%s, service:%s", type, host, service);
    if (!strcmp(type,CLIENT_TYPE_INET_UDP)) {
        client = OBJECT_NEW(allocator, Inet_Udp_Client, NULL);
        if (service != NULL) client->bind(client, host, service); 
    } else if (!strcmp(type,CLIENT_TYPE_INET_UDP_V6)) {
        client = OBJECT_NEW(allocator, Inet_Udp_V6_Client, NULL);
        if (service != NULL) client->bind(client, host, service); 
    } else if (!strcmp(type,CLIENT_TYPE_INET_TCP)) {
        client = OBJECT_NEW(allocator, Inet_Tcp_Client, NULL);
    } else {
        dbg_str(NET_WARN,"client error type");
        return NULL;
    }

    return (void *)client;
}

int client_connect(void *client, char *host, char *service)
{
    Client *c = (Client *)client;
    return c->connect(c, host, service);
}

int client_trustee(void *client, struct timeval *tv, void *work_callback, void *opaque)
{
    Client *c = (Client *)client;
    int ret;

    TRY {
        // THROW_IF(work_callback == NULL, -1);
        c->trustee(c, NULL, (void *)work_callback, opaque);
    } CATCH (ret) {
    }

    return ret;
}

int client_send(void *client, void *buf, int len, int flags)
{
    Client *c = (Client *)client;
    return c->send(c, buf, len, flags);
}

int client_destroy(void *client)
{
    return object_destroy(client);
}

void *server(allocator_t *allocator, char *type,
             char *host, char *service,
             int (*work_callback)(void *arg),
             void *opaque)
{
    Server *server = NULL;
    int ret;

    TRY {
        if (!strcmp(type, SERVER_TYPE_INET_TCP)) {
            THROW_IF(work_callback == NULL, -1);
            server = OBJECT_NEW(allocator, Inet_Tcp_Server, NULL);
            EXEC(server->bind(server, host, service)); 
            EXEC(server->trustee(server, (void *)work_callback, opaque));
        } else {
            dbg_str(NET_WARN,"server type error");
            return NULL;
        }
    } CATCH (ret) {
        object_destroy(server);
        server = NULL; 
    }

    return (void *)server;
}

int server_destroy(void *server)
{
    return object_destroy(server);
}