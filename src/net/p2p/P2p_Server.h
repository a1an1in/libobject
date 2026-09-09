#ifndef __STUN_SERVER_H__
#define __STUN_SERVER_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/concurrent/net/api.h>
#include "stun/stun_header.h"

typedef struct P2p_Server_s P2p_Server;

/* 地址簿项：peer id → 两个地址（客户端把“打洞口”与“信令口”分开绑定）：
 *  - punch_host/punch_port = 打洞(公网/公告)地址：REG 报文里 peer 上报的公网映射，
 *    服务器把它转给对端（INVITE/PEER 文本），对端据此向该 peer 打洞；
 *  - signal_host/signal_port = 信令地址：peer 发 REG 这个包的源地址（= peer 的信令
 *    socket，服务器必可达，如与服务器同机/LAN）。服务器“主动通知”该 peer 的
 *    INVITE/CONNECTED 投到它，避免发 peer 自己公网映射时 hairpin 不通。
 * 兼容单 socket 旧实现：未上报公告地址时两者相同（都用观测源）。 */
typedef struct stun_server_peer_s {
    char id[32];
    char punch_host[64];   /* 打洞/公网地址（公告映射，给对端打洞） */
    int  punch_port;
    int  nat_type;         /* peer 上报的 NAT 类型，取值见 Stun.h 枚举 */
    char signal_host[64];  /* 信令地址（REG 源，服务器通知该 peer 用） */
    int  signal_port;
} stun_server_peer_t;

/* 撮合中的呼叫：key = 发起方 id，值为其想连的对端与双方地址。
 * caller_ok/callee_ok：发起方/被叫方是否已上报“打洞成功”；双方都上报，
 * 服务器才判定两端 ok 并回双方 CONNECTED（ACCEPT 只是愿意配合打洞）。 */
typedef struct p2p_server_call_s {
    char from_id[32];   /* 发起方 id（map key） */
    char from_host[64]; /* 发起方公告/观测地址 */
    int  from_port;
    char to_id[32];     /* 被叫方 id */
    char to_host[64];   /* 被叫方地址(撮合成功后回给发起方) */
    int  to_port;
    int  to_nat;
    int  caller_ok;     /* 发起方打洞成功已上报 */
    int  callee_ok;     /* 被叫方打洞成功已上报 */
} p2p_server_call_t;

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
struct P2p_Server_s{
    Obj obj;

    int (*construct)(P2p_Server *, char *);
    int (*deconstruct)(P2p_Server *);
    int (*set)(P2p_Server *, char *attrib, void *value);
    void *(*get)(void *, char *attrib);
    int (*start)(P2p_Server *server, char *host, char *service);
    int (*stop)(P2p_Server *server);

    Client *client;   /* UDP client：bind 端口，client_trustee 收包 */
    Map *peers;       /* key: peer id 字符串, value: stun_server_peer_t* */
    Map *pending;     /* 撮合中呼叫: key=发起方 id, value: p2p_server_call_t* */
    pthread_mutex_t lock;
};

#endif
