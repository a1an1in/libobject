#ifndef __STUN_SERVER_H__
#define __STUN_SERVER_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/concurrent/net/api.h>
#include "stun_header.h"

typedef struct Stun_Server_s Stun_Server;

/* 地址簿项：peer id → 公网映射地址 */
typedef struct stun_server_peer_s {
    char id[32];
    char host[64];
    int port;
} stun_server_peer_t;

/*
 * STUN 服务器（兼任地址交换/信令）。
 *
 * 基于 UDP client（client + client_trustee 异步收包），一个端口同时提供：
 *  - STUN Binding（RFC 5389）：回显请求者的公网映射地址（XOR-MAPPED-ADDRESS）
 *  - 地址簿：收到文本 "REG <id>" 记录请求者源地址；"GET <id>" 返回对端地址
 * 地址簿用 Map（RBTree，key=id 字符串）存储，支持任意数量的 peer。
 *
 * 部署在公网。peer（Stun 统一客户端）连上后即可：查自己映射地址、
 * 注册自己、查询对端、打洞。
 */
struct Stun_Server_s{
    Obj obj;

    int (*construct)(Stun_Server *, char *);
    int (*deconstruct)(Stun_Server *);
    int (*set)(Stun_Server *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);
    int (*start)(Stun_Server *server, char *host, char *service);
    int (*stop)(Stun_Server *server);

    Client *client;   /* UDP client：bind 端口，client_trustee 收包 */
    Map *peers;       /* key: peer id 字符串, value: stun_server_peer_t* */
    pthread_mutex_t lock;
};

#endif
