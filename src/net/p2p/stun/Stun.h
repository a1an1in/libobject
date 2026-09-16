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
typedef struct stun_session_s stun_session_t;

/* NAT 类型（Stun.probe 探测）。判定只需区分是否对称：
 * 任一端非对称(端口稳定)即可打洞，两端都对称才需 TURN。 */
enum {
    STUN_NAT_TYPE_UNKNOWN    = 0,  /* 未探测 / 未知 */
    STUN_NAT_TYPE_OPEN,            /* 无 NAT，直连公网（可打洞） */
    STUN_NAT_TYPE_CONE,            /* 非对称：外部端口稳定（可打洞） */
    STUN_NAT_TYPE_SYMMETRIC,       /* 对称型：每新目的换端口（需 TURN） */
};

/* 会话状态 */
enum {
    STUN_SESSION_IDLE     = 0,     /* 已建(含 data socket)，采址中/未发起 */
    STUN_SESSION_MAKING,           /* 本会话 data socket 采址完成，INVITE 已发，等 reply */
    STUN_SESSION_RINGING,          /* 被叫：收到 INVITE，采址完成 INVITE_REPLY(accept) 已发 */
    STUN_SESSION_PUNCHING,         /* 已拿到对端会话地址，互发 KEEPALIVE 打洞中 */
    STUN_SESSION_CONNECTED,        /* 服务器 CONNECTED：双方打洞成功，可通信 */
    STUN_SESSION_CLOSED,           /* 已关闭/待删除 */
};

/*
 * 会话（stun_session）：与某个对端 stun 节点的一条链路。session 由 Stun(节点)统一管理：
 *  - 本端主叫 call(remote stun id) 时建；
 *  - 本端被叫收到服务器 INVITE 时建。
 * 每条会话**独立**持有自己的 UDP data socket（随机端口、不 connect、sendto），打洞/
 * 保活/采址/业务数据都走各自会话 socket。会话值 allocator_mem_alloc 分配，放入
 * stun->sessions(Map, remote stun id -> session)，由 Map trustee(reset) 统一释放
 * （del 后不得再引用）。
 */
struct stun_session_s {
    char remote_id[32];            /* 对端 stun id（=会话表 key） */

    /* 本会话数据面：独立 UDP socket（不 connect、sendto；原节点级 peer_client 迁入） */
    Client *peer_client;
    int   local_port;                   /* 本会话占用的本地端口（0=随机口/未用池）；
                                         * 非 0 时来自节点端口池，__destroy_session 归还 */
    char  own_host[64]; int own_port;   /* 本会话公网地址（本 socket 采址结果） */
    char  peer_host[64]; int peer_port; /* 对端本会话地址（信令交换所得，打洞目标） */
    int   peer_nat_type;                /* 对端 NAT（撮合回执带回，可暂不填） */
    int   nat_type;                     /* 本端 NAT：discovery 双目的地探测，STUN_NAT_TYPE_* */

    /* 本会话身份/角色：caller=本端主叫发起；callee=被叫(收到 INVITE 建)。 */
    int role;                    /* 0=caller, 1=callee */
    int own_ready;               /* 本会话 own 公网地址已采址就绪 */

    /* 链路状态（state 为权威，connected 为其等价缓存） */
    int state;                   /* STUN_SESSION_* */
    int connected;
    int send_punch;              /* 建链：收到对端包后上报一次 PUNCHOK */
    int data_received;           /* 收到对端任一 P2P 包(保活/数据) */
    int active;                  /* 会话在用/在表内 */

    /* 本会话保活：事件定时器（事件线程驱动，非线程） */
    void *keepalive_worker;
    int   keepalive_interval_ms;

    /* 本会话业务收包回调与上下文（DATA 到达时优先调它；NULL 则走节点 recv_callback）。
     * 由上层(如 p2p)在会话创建/收到数据时设置，实现按会话区分业务处理。 */
    int (*recv)(void *opaque, const uint8_t *data, int len);
    void *opaque;

    /* 本会话采址(STUN Binding 解析)独立对象——多会话并发采址不共享节点对象 */
    Request *req;
    Response *response;

    Stun *stun;                    /* 回指所属节点 */
};

/*
 * P2P UDP 消息（对端打洞/保活共用同一 KEEPALIVE 报文，业务数据 DATA 报文；
 * 区别于 STUN 报文）。收到任一 P2P 报文都证明“对端→本端”可达。 */
#define STUN_P2P_MAGIC        0x50325021   /* 'P2P!' */
#define STUN_P2P_MSG_KEEPALIVE 1           /* 保活/打洞（建链期与链路上统一用） */
#define STUN_P2P_MSG_DATA     2            /* 业务数据 */
#define STUN_P2P_MAX_PAYLOAD  1400

/* 会话 data socket 本地端口池容量：local_service 写"端口列表/范围"时最多解析出这么多
 * 端口，也就是本节点最多用固定端口同时支撑这么多条链路（超出部分回落随机口）。 */
#define STUN_SERVICE_POOL_MAX 32

/* INVITE_REPLY 里 reject 的原因码（被叫/服务器带回，主叫据此区分失败原因）：
 *   INVITE_REPLY <callee> <caller> reject <reason>  —— 全链透传（被叫->服务器->主叫） */
#define STUN_REJECT_NONE      0   /* 未说明原因；作为 reply_session_request 的参数时 = 接受 */
#define STUN_REJECT_NO_PORT   1   /* 本地数据口端口池已满（local_service 容量不够） */
#define STUN_REJECT_INTERNAL  2   /* 其他内部原因（建会话失败等） */
#define STUN_REJECT_OFFLINE   3   /* 对端不在线（服务器生成） */

typedef struct stun_p2p_msg_s {
    uint32_t magic;    /* STUN_P2P_MAGIC（网络序） */
    uint8_t type;      /* KEEPALIVE / DATA */
    uint16_t len;      /* data 长度（网络序） */
    uint8_t data[0];
} stun_p2p_msg_t;

/*
 * Peer 统一客户端（RFC 5389 STUN + P2P 打洞）。
 *
 * 一个 Stun 对象即一个**节点**（上线后以 stun_id 被寻址）：
 *  - server_client：与信令服务器的常驻会话(信令口)，只收发服务器信令文本
 *    （SIGNIN/OK、INVITE、INVITE_REPLY(accept|reject)、PUNCHOK/CONNECTED、SIGNOUT），
 *    回调 __stun_signal_callback；
 *  - sessions：会话表(remote stun id -> stun_session_t)。每条会话一个独立 UDP data
 *    socket(peer_client，绑 local_host + 随机口，不 connect、sendto)，负责采址/打洞/
 *    保活/业务数据，回调各自归属会话。
 * 被叫 INVITE/CONNECTED 由服务器投到信令口(server_client)；会话地址(own)经信令交换：
 * 主叫 INVITE 带本会话地址，被叫 INVITE_REPLY(accept) 带本会话地址，服务器把这条 reply
 * 转给主叫从而带回对端会话
 * 地址，两端据此打洞。免 GET/免节点级公告打洞地址。
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
     * 与信令服务器建立常驻连接（server_client），
     * 供后续 SIGNIN/INVITE/INVITE_REPLY/PUNCHOK/SIGNOUT。
     * @param host    服务器地址
     * @param service 服务器端口
     */
    int (*connect)(Stun *stun, char *host, char *service);
    /*
     * 向信令服务器注册本节点（SIGNIN <stun_id>，同步等待 OK）。
     * 服务器记录 SIGNIN 源地址作为本节点信令地址（INVITE/CONNECTED 投递用）。
     * @param stun_id 本节点 stun id
     */
    int (*signin)(Stun *stun, char *stun_id);
    /*
     * 设置本节点采址(discovery)用的公共 STUN 服务器地址（会话 data socket 采址用）。
     * 不设置则回退到信令服务器自身（loopback/同机）。
     */
    int (*set_stun_server)(Stun *stun, char *host, char *service);
    /* 下线(SIGNOUT)并停全部会话；节点句柄仍可用但已离线。返回 0=成功。 */
    int (*signout)(Stun *stun);
    /* 注册业务收包回调（DATA 到达任意会话时调用，session 标识对端）。 */
    int (*set_recv_callback)(Stun *stun,
                             int (*func)(Stun *stun, stun_session_t *session,
                                         uint8_t *buf, int len));

    /* ---- 会话级：名字含 session 的接口集中在这里（remote id 定位会话） ---- */
    /*
     * 建/复用一条会话并发起采址（合并原 call/create）：
     *  - role：0=caller 主叫，1=callee 被叫；
     *  - 分配会话(独立 data socket + req/response)入表；已存在同 remote 会话则复用；
     *  - 随后发起采址，own 就绪由回调按 role 调 request_session/reply_session_request；
     *  - out 非空回传会话指针(免调用者再 get)。返回 0=成功；-1=失败。
     */
    int (*create_session)(Stun *stun, char *remote_stun_id, int role,
                          stun_session_t **out);
    /* 关闭某会话：停保活、关 data socket、从会话表移除。返回 0=成功。 */
    int (*close_session)(Stun *stun, char *remote_stun_id);
    /* 按 remote stun id 查会话（无则返回 NULL）。 */
    stun_session_t *(*get_session)(Stun *stun, char *remote_stun_id);
    /* 向 remote stun id 会话发业务数据（DATA，走会话 data socket）。返回 0=成功；-1=失败。 */
    int (*send_session_data)(Stun *stun, char *remote_stun_id, void *buf, int len);
    /* 发起采址(STUN Binding，异步)；own 就绪后回调按角色 request_session/reply_session_request。 */
    int (*probe_session_addr)(Stun *stun, char *remote_stun_id);
    /* 会话请求（主叫）：采址就绪后发 INVITE <my> <callee> <own_host> <own_port> <nat>。 */
    int (*request_session)(Stun *stun, char *remote_stun_id);
    /* 会话应答（被叫）：对收到的 INVITE 回 INVITE_REPLY（accept/reject 二合一，
     * 用一个原因码区分）：
     *   reason == STUN_REJECT_NONE(0) -> 接受(accept)，带本会话地址；
     *   reason != 0                   -> 拒绝(reject)，只带原因码（如 STUN_REJECT_NO_PORT）。
     * 与 request_session 成对：一个发 INVITE 请求，一个回 INVITE_REPLY 应答。 */
    int (*reply_session_request)(Stun *stun, char *remote_stun_id, int reason);
    /* 开始打洞：设定对端会话地址并周期互发 KEEPALIVE。 */
    int (*punch_session)(Stun *stun, char *remote_stun_id,
                         char *peer_host, int peer_port);

    /* 查询会话是否已打通：0=连通；-1=未通/无会话（名字不含 session，故紧跟会话组）。 */
    int (*is_connected)(Stun *stun, char *remote_stun_id);

    /* 本地绑定地址（peer/data socket 绑 host；字符串由调用方持有） */
    char *local_host;
    /* peer/data socket 本地端口**池**（字符串由调用方持有）。三种写法：
     *   "12346"            单端口（=只支持 1 条链路）；
     *   "12346,12347,12348" 逗号列表（=支持 3 条链路）；
     *   "12346-12350"       范围（含端点，=支持 5 条链路）；可与逗号混写。
     * 每建一条会话就从池里取一个**空闲**端口（真的绑不上会自动换下一个）；
     * 池耗尽或为 NULL -> 该链路改用随机口（等价"不指定端口"）。 */
    char *local_service;
    /* ---- 上面 local_service 解析出来的端口池（惰性解析一次） ---- */
    int   pool_parsed;                              /* 0=还没解析过 */
    int   pool_num;                                 /* 池内端口个数（=容量）；0=未配/解析失败 */
    int   pool_ports[STUN_SERVICE_POOL_MAX];
    int   pool_used[STUN_SERVICE_POOL_MAX];         /* 1=已被某会话占用 */
    /* 信令服务器地址（connect 时保存） */
    char *signal_host;
    char *signal_service;
    /* 公共 STUN 采址服务器（set_stun_server 设置；缺省用信令服务器） */
    char *stun_host;
    char *stun_service;
    /* 第二 STUN 采址服务器（对称探测用；直接赋值即可，可配成信令服——它兼 STUN 回显）。
     * 不设置则不做 nat 探测(nat_type 保持 UNKNOWN)。 */
    char *stun2_host;
    char *stun2_service;

    /* 本节点 stun id（signin 记录） */
    char stun_id[32];

    /* 与信令服务器的常驻会话（信令口，收 INVITE_REPLY/CONNECTED 等文本） */
    Client *server_client;
    /* 会话表：remote stun id -> stun_session_t* */
    Map *sessions;

    /* 信令同步等待（SIGNIN 等 OK） */
    int register_done;

    /* 业务收包回调（DATA 任意会话到达时上抛）与上下文 */
    int (*recv_callback)(Stun *stun, stun_session_t *session, uint8_t *buf, int len);
    void *opaque;

    /* 默认保活周期（会话建时继承；未设则内部用 1000ms） */
    int keepalive_interval_ms;
};

typedef struct attrib_parse_policy_s {
    int type;
    int (*policy)(stun_attrib_t *, stun_attrib_t *);
} attrib_parse_policy_t;

extern attrib_parse_policy_t g_stun_parse_attr_policies[];
extern int g_stun_parse_attr_policies_count;

#endif
