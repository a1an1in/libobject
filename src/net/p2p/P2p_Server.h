#ifndef __STUN_SERVER_H__
#define __STUN_SERVER_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/concurrent/net/api.h>
#include "stun/stun_header.h"

typedef struct P2p_Server_s P2p_Server;

/*
 * 在线 stun 节点登记：stun id -> 信令地址（REG 源地址，服务器必可达）。
 * v3 多会话模型下，打洞目标(会话地址)不再在 REG 上报，而是随 CALL/ACCEPT 信令
 * 交换；因此登记表只记信令地址，用于给该节点投递 INVITE/MATCH/CONNECTED/NOPEER。
 */
typedef struct stun_node_s {
    char id[32];
    char signal_host[64];   /* 信令地址（REG 源，服务器通知该节点用） */
    int  signal_port;
} stun_node_t;

/*
 * 撮合中的呼叫：key = "caller_id|callee_id"（支持一节点同时多路撮合）。
 *  - from_host/from_port = 主叫 CALL 的信令源地址（撮合回执 MATCH/CONNECTED 投递）
 *  - to_host/to_port     = 被叫 ACCEPT 带回的【本会话打洞地址】（MATCH 给主叫打洞用）
 * 两端是否打通以各自 PUNCHOK 上报为准（caller_ok/callee_ok 都置位才判定 ok，
 * 回双方 CONNECTED；ACCEPT 只是"愿意配合打洞"，非成功）。
 */
typedef struct p2p_server_call_s {
    char key[96];        /* "caller_id|callee_id"，map key */
    char from_id[32];    /* 主叫 stun id */
    char from_host[64];  /* 主叫信令地址（服务器回执 MATCH/CONNECTED 投递） */
    int  from_port;
    char to_id[32];      /* 被叫 stun id */
    char to_host[64];    /* 被叫本会话打洞地址(ACCEPT 带回，MATCH 用) */
    int  to_port;
    int  from_nat;       /* 主叫 NAT(CALL 带回，INVITE 转给被叫) */
    int  to_nat;         /* 被叫 NAT(ACCEPT 带回，MATCH 转给主叫) */
    int  caller_ok;      /* 主叫已打洞成功上报 */
    int  callee_ok;      /* 被叫已打洞成功上报 */
} p2p_server_call_t;

/*
 * STUN/信令服务器（中心）。
 *
 * 一个端口同时提供：
 *  - STUN Binding（RFC 5389）：回显请求者公网映射（XOR-MAPPED-ADDRESS）
 *  - 信令：REG/BYE 登记 + CALL/INVITE/ACCEPT/PUNCHOK 撮合（会话地址交换）
 * 登记与撮合用 Map(RBTree, 字符串 key)。部署在公网；stun 节点连上即可 REG、
 * 撮合、被叫等 INVITE。
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
    Map *stuns;       /* key: stun id, value: stun_node_t* */
    Map *pending;     /* 撮合中呼叫: key "caller|callee", value: p2p_server_call_t* */
    pthread_mutex_t lock;
};

#endif
