#ifndef __NET_WORKER_API_H__
#define __NET_WORKER_API_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/Obj.h>
#include <libobject/core/io/Socket.h>
#include <libobject/concurrent/worker_api.h>
#include <libobject/concurrent/net/Client.h>
#include <libobject/concurrent/net/Server.h>

#define SERVER_TYPE_INET_TCP "inet_tcp_server_type"
#define SERVER_TYPE_UNIX_TCP "tcp_userver_type"

#define CLIENT_TYPE_INET_TCP "inet_tcp_client_type"
#define CLIENT_TYPE_INET_UDP "inet_udp_client_type"
#define CLIENT_TYPE_UNIX_TCP "unix_tcp_client_type"
#define CLIENT_TYPE_UNIX_UDP "unix_udp_client_type"

void *client(allocator_t *allocator, char *type,
             char *host, char *service);
int client_connect(void *client, char *host, char *service);
int client_trustee(void *client, struct timeval *tv, void *work_callback, void *opaque);
int client_send(void *client, void *buf, int len, int flags);
int client_destroy(void *client);

void *server(allocator_t *allocator, 
             char *type,
             char *host,
             char *service,
             int (*process_task_cb)(void *arg),
             void *opaque);
int server_destroy(void *server);

#endif
