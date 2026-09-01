#ifndef __STUN_H__
#define __STUN_H__

#include <stdio.h>
#include <pthread.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Map.h>
#include <libobject/concurrent/net/api.h>
#include "stun_header.h"
#include "Request.h"
#include "Response.h"

typedef struct Stun_s Stun;

/* 对端地址 */
typedef struct stun_peer_addr_s {
    char host[64];
    int port;
} stun_peer_addr_t;

/* P2P UDP 消息（打洞/保活/业务数据共用，区别于 STUN 报文） */
#define STUN_P2P_MAGIC        0x50325021   /* 'P2P!' */
#define STUN_P2P_MSG_PUNCH    1            /* 打洞 */
#define STUN_P2P_MSG_KEEPALIVE 2           /* 保活 */
#define STUN_P2P_MSG_DATA     3            /* 业务数据 */
#define STUN_P2P_MAX_PAYLOAD  1400

typedef struct stun_p2p_msg_s {
    uint32_t magic;    /* STUN_P2P_MAGIC（网络序） */
    uint8_t type;      /* PUNCH / KEEPALIVE / DATA */
    uint16_t len;      /* data 长度（网络序） */
    uint8_t data[0];
} stun_p2p_msg_t;

/*
 * Peer 统一客户端（RFC 5389 STUN + P2P 打洞）。
 *
 * 一个 Stun 对象即一个 peer 节点。STUN 查询（映射地址）、信令（向中心
 * 服务器注册/查询对端）、打洞、保活、业务数据收发共用同一个 UDP socket
 * （stun->c），保证 NAT 映射端口一致。内部收包回调统一分发：
 *  - STUN Binding 响应 → 解析为 mapped_host/mapped_port
 *  - 中心服务器信令回复（OK / PEER host port）→ 更新注册/查询结果
 *  - P2P 消息（PUNCH/KEEPALIVE/DATA）→ 处理/回调业务数据
 */
struct Stun_s{
    Obj parent;

    int (*construct)(Stun *,char *);
    int (*deconstruct)(Stun *);

    /*virtual methods reimplement*/
    int (*set)(Stun *module, char *attrib, void *value);
    void *(*get)(Stun *, char *attrib);
    char *(*to_json)(Stun *); 
    /*
     * 创建 UDP client（本地绑定地址由 stun->local_host/local_service 决定），
     * 并连接中心服务器（STUN+信令一体），注册收包回调。
     * @param host    服务器地址
     * @param service 服务器端口
     */
    int (*connect)(Stun *stun, char *host, char *service);
    /*
     * 向中心服务器发起 STUN Binding Request（异步）。
     * 响应到达后由内部回调解析出本 peer 公网映射地址写入 mapped_host/mapped_port。
     */
    int (*discovery)(Stun *stun);
    /*
     * 向中心服务器注册本 peer（同步等待确认）。
     * 服务器以 UDP 源地址记录本 peer 的公网映射地址。
     * @param id 本 peer 的 id
     */
    int (*register_addr)(Stun *stun, char *id);
    /*
     * 向中心服务器查询对端地址（同步等待结果）。
     * @param peer_id  对端 id
     * @param host     输出对端公网地址
     * @param host_len host 缓冲区大小
     * @param port     输出对端端口
     */
    int (*lookup_addr)(Stun *stun, char *peer_id, char *host, int host_len, int *port);
    /*
     * 向对端打洞：UDP connect 切换到对端地址并发打洞包。
     * @param host    对端公网地址
     * @param service 对端端口
     */
    int (*punch)(Stun *stun, char *host, char *service);
    /* 向当前对端（最近 punch 切换的地址）发送业务数据 */
    int (*send)(Stun *stun, void *buf, int len);
    /* 启动保活线程，周期性向对端发送 keepalive 包 */
    int (*keepalive_start)(Stun *stun, int interval_ms);
    int (*keepalive_stop)(Stun *stun);
    /* 注册业务数据接收回调（STUN 响应与信令回复内部处理，不回调） */
    int (*set_recv_callback)(Stun *stun, int (*func)(Stun *stun, uint8_t *buf, int len));

    /* 本地绑定地址（字符串由调用方持有，本对象不管理生命周期） */
    char *local_host;
    char *local_service;
    /* 中心服务器地址（connect 时保存，discovery 重连回服务器使用） */
    char *stun_host;
    char *stun_service;
    /* UDP client（STUN 查询 + 信令 + P2P 打洞/保活/数据共用） */
    Client *c;
    /* STUN 公网映射地址查询结果 */
    char mapped_host[64];
    int mapped_port;
    /* 对端地址（最近 punch 目标） */
    stun_peer_addr_t remote;
    /* 业务数据接收回调 */
    int (*recv_callback)(Stun *stun, uint8_t *buf, int len);
    void *opaque;

    /* 信令查询结果（同步等待用） */
    int register_done;
    int lookup_done;
    char lookup_host[64];
    int lookup_port;

    /* 保活线程 */
    pthread_t keepalive_thread;
    int keepalive_interval_ms;
    int stop;

    Request *req;
    Response *response;
};

typedef struct attrib_parse_policy_s {
    int type;
    int (*policy)(stun_attrib_t *, stun_attrib_t *);
} attrib_parse_policy_t;

extern attrib_parse_policy_t g_stun_parse_attr_policies[];
extern int g_stun_parse_attr_policies_count;

#endif
