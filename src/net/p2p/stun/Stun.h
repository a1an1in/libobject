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

/* NAT 类型（Stun.probe 探测）。判定只需区分是否对称：
 * 任一端非对称(端口稳定)即可打洞，两端都对称才需 TURN。 */
enum {
    STUN_NAT_TYPE_UNKNOWN    = 0,  /* 未探测 / 未知 */
    STUN_NAT_TYPE_OPEN,            /* 无 NAT，直连公网（可打洞） */
    STUN_NAT_TYPE_CONE,            /* 非对称：外部端口稳定（可打洞） */
    STUN_NAT_TYPE_SYMMETRIC,       /* 对称型：每新目的换端口（需 TURN） */
};

/* 对端地址 */
typedef struct stun_peer_addr_s {
    char host[64];
    int port;
} stun_peer_addr_t;

/* P2P UDP 消息（对端打洞/保活共用同一 KEEPALIVE 报文，业务数据 DATA 报文；
 * 区别于 STUN 报文）。收到任一 P2P 报文都证明“对端→本端”可达。 */
#define STUN_P2P_MAGIC        0x50325021   /* 'P2P!' */
#define STUN_P2P_MSG_KEEPALIVE 1           /* 保活/打洞（建链期与链路上统一用） */
#define STUN_P2P_MSG_DATA     2            /* 业务数据 */
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
    /*
     * 向指定的 STUN 服务器（如 stun.cloudflare.com:3478）发 Binding，用同一个
     * UDP socket 取得本 peer 的真实公网映射地址（写入 mapped_host/mapped_port），
     * 随后自动切回信令中心(signal_host/service)，供后续 REG/GET 使用。
     * @param host    STUN 服务器地址（可与信令中心不同）
     * @param service STUN 服务器端口
     */
    int (*discovery)(Stun *stun, char *host, char *service);
    /*
     * 向信令中心注册本 peer：把 discovery 得到的公网映射地址主动上报
     * （格式 "REG <id> <host> <port>"），服务器登记该公告地址（而非其观测
     * 到的源地址），供对端查询后打洞。
     * @param id 本 peer 的 id
     */
    int (*register_addr)(Stun *stun, char *id);
    /*
     * 探测 NAT 是否对称：用同一个 UDP socket 向两个不同的 STUN(hostA/hostB)
     * 采址，比较两次外部映射端口，结果写入 stun->nat_type。
     * 返回：1=非对称(端口稳定，可打洞)；0=对称型(需 TURN)；-1=失败。
     */
    int (*probe)(Stun *stun, char *hostA, char *serviceA,
                 char *hostB, char *serviceB);
    /*
     * 主叫(发起方)：向 peer_id 发起连接(异步)。内部发 CALL，收到服务器 PEER 后
     * 自动打洞并复用保活线程发 PUNCH，收到对端包后内部上报 PUNCHOK，服务器回
     * CONNECTED 后置 stun->connected。返回 0=已发出 CALL。
     */
    int (*connect_peer)(Stun *stun, char *peer_id);
    /*
     * 向服务器(server_client)发送一行信令文本命令（REG/CALL/ACCEPT/PUNCHOK/BYE…
     * 自动加 '\n'）。返回 0=成功；-1=失败。
     */
    int (*send_server)(Stun *stun, const char *line);
    /*
     * 向指定对端地址(host:service)经 peer_client sendto 一包（不 connect；
     * 打洞/保活/采址等原始 P2P 报文）。返回 0=成功；-1=失败。
     */
    int (*send_peer)(Stun *stun, char *host, char *service, void *buf, int len);

    /* 本地绑定地址（字符串由调用方持有，本对象不管理生命周期） */
    char *local_host;
    char *local_service;
    /*
     * 信令中心地址（connect 时保存；REG/GET/公告地址都发给它）。
     * 与 discovery 用的公共 STUN 地址是两个不同的地址。
     */
    char *signal_host;
    char *signal_service;
    /*
     * 两个 UDP client（信令口与打洞口分开，各自独立收包回调）：
     *  - server_client：信令(控制) client。绑临时端口，connect() 常驻中心服务器，
     *    只收发服务器信令（REG/GET/CALL/ACCEPT/PUNCHOK/BYE 与 OK/WAIT/PEER/
     *    NOPEER/CONNECTED 回复），收包回调 = __stun_signal_callback；
     *  - peer_client：P2P(打洞/数据) client。绑 local_service（公告/打洞地址，
     *    对端打洞与服务器 INVITE/CONNECTED 都投到这个地址），它保持【不 connect】，
     *    与对端/公共 STUN 的收发一律 sendto，故既能收到对端 P2P 包，也能在打洞
     *    期间收到服务器投递到公告地址的信令（被叫侧 INVITE/CONNECTED），
     *    不会被内核按 connect 目标过滤。收包回调 = __stun_peer_callback。
     */
    Client *server_client;
    Client *peer_client;
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
    /* NAT 类型：stun->nat_type 为本端(probe 探测)，peer_nat_type 为对端
     * (由信令 PEER 回复带回)。取值见顶部枚举。 */
    int nat_type;
    int peer_nat_type;
    /* stun_peer_run 使用：收到对端 DATA 后置 1（内部接收包装更新） */
    int data_received;

    /* 撮合/建链状态（Stun 内部处理；connect/listen 上层只读 connected） */
    char id[32];          /* 本端注册 id（register_addr 记录） */
    char peer_id[32];     /* 当前呼叫对端：主叫=目标，被叫=邀请方 */
    int  connected;       /* 服务器 CONNECTED：双方打洞都成功，可通信 */
    int  dialing;         /* 主叫：已发 CALL，等 PEER */
    int  send_punch;      /* 建立期：保活线程未收到对端包时发 PUNCH，收到后上报 PUNCHOK */

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
