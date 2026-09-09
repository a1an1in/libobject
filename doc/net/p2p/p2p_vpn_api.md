# p2p 供 VPN 使用的常驻会话接口设计

> 目的：让上层(首个使用者 VPN，也可作一般常驻点对点)能“建链成功后长期保持通道、
> 随时收发业务包”。基于 net/p2p 打洞客户端(Stun)的能力，以**不透明会话句柄**
> 暴露到公共头 `src/include/libobject/net/p2p/p2p.h`。
> 状态：接口设计稿（实现另起 code 任务）。

## 1. 目标与非目标

目标：
- 一次建链得到**可长期持有**的会话：底层由事件/线程维持通道(含 keepalive)；
- 上层可随时 `send`，收到对端业务包通过注册回调获得；
- VPN 只需要公共头，不接触内部 Stun。
非目标：本设计不含 TURN 中继接入、多 peer 路由；这些后续。

## 2. 概念与线程模型

- 建链、保活、收包由 p2p 会话内部维护（复用 Stun + 其 client 事件线程/keepalive 线程）。
- `p2p_session_open` 是**同步建链**（内部完成 discovery/probe/register/lookup/punch，
  超时/失败返回负值）。成功即返回**已就绪**会话（on_ready 不再需要——已就绪；
  见下方说明）。建链等待沿用现底层同步流程。
- 建链成功后：会话**长驻**，直到 `p2p_session_close`。
  - 业务包到达 → 内部 Stun 收包回调 → 分发给用户注册的 recv 回调；
  - keepalive 由内部线程周期发送，用户不感知。

> 说明：虽然 stun_peer_run“成功即退”，会话接口刻意**不自己退出**，由 close 显式结束，
> 以支撑 VPN 常驻；一次一包发收见 API 语义。

## 3. 对外 API（公共头草案）

```c
/* ===== 会话 ===== */
typedef struct p2p_session_s p2p_session_t;      /* 不透明会话句柄 */

/* 业务收包回调：data/len 仅在回调内有效 */
typedef int (*p2p_recv_fn)(void *opaque, const uint8_t *data, int len);

/* 建链并持有常驻会话，保证两端都就绪后才成功：
 * cfg 沿用 p2p_cfg_t（id/peer_id/local/signal/stun/stun2/…；recv 等由下方单独注册）。
 * 打洞后做双向就绪握手：按 interval 重发直到收到对端任一 P2P 消息，确认两端
 * session 都成功、链路双向可达，才返回 0(可安全 send)；超时返回 -3。
 * 失败返回负值，*s 置 NULL。建链内部使用 cfg.timeout_ms 作最长等待。
 */
int p2p_session_open(p2p_session_t **s, p2p_recv_fn recv,
                     const p2p_cfg_t *cfg, void *opaque);

/* 向对端发送一个业务包（内部封装 P2P DATA）。返回 0 成功；负值失败。
 * 可在任意线程调用（内部对写做加锁）。 */
int p2p_session_send(p2p_session_t *s, const uint8_t *data, int len);

/* 关闭会话：停保活、释放内部资源与句柄。幂等；之后 *s 不可再用。 */
int p2p_session_close(p2p_session_t *s);

/* 查询会话状态（可选）：0=已连接，非0=异常/已关闭等。 */
int p2p_session_is_alive(p2p_session_t *s);
```

错误码：
- 0   = 两端已就绪（可安全 send）；
- -1  = 失败；
- -2  = 双对称 NAT 需 TURN（暂未接入，上层可感知后决定重试或放弃）；
- -3  = 对端未在 timeout_ms 内确认（双向未就绪，不要 send）。

## 4. 与现有接口的关系

- `p2p_session_*`：长驻通道（VPN/长期上层使用），为对外主接口。
- `p2p_server_run`：中心服务器运行入口。
- 不再提供一次性 `p2p_peer_run`；一次性演示由调用方用 open→发/收→close 自行组织。

## 5. VPN 侧集成时序（示例伪代码）

```c
static int vpn_recv(void *opaque, const uint8_t *d, int len)
{
    Tun *tun = opaque;            /* vpn 上下文 */
    return tun->write(tun, d, len);   /* 对端包注入本机 */
}

/* 建链 */
p2p_session_t *s = NULL;
ret = p2p_session_open(&s, vpn_recv, &p2p_cfg, tun);
if (ret < 0) { /* -2 需 TURN：可重试或放弃 */ return ret; }

/* 转发主循环：阻塞读 tun -> send（点对点） */
for (;;) {
    n = tun->read(tun, buf, sizeof(buf));
    if (n <= 0) { if (errno==EINTR && break_flag) break; continue; }
    p2p_session_send(s, buf, n);
}
p2p_session_close(s);
```

要点：
- 入站完全靠 `p2p_session_open` 注册的 recv 回调（无需 VPN 线程）；VPN 只需一个
  阻塞读 tun 的转发线程 + Ctrl+C 退出。
- 出站 `p2p_session_send` 可跨线程（内部写锁）。
- VPN 不用感知 keepalive。

## 6. 实现要点（代码任务清单）

1. p2p.c 增加 p2p_session 结构（持 Stun*、recv/opaque、写锁）与上述 4 个函数；
   - open：Stun 建链（连接/采址/探测/注册/查对端/打洞）→ 注册收包回调(桥接 recv)
     → 启动 keepalive；返回句柄。
   - send：`stun->send`；close：停 keepalive + destroy Stun + 置空。
2. 桥接收包：内部 Stun recv 回调拿到 (Stun*, buf, len) → 调用户 (opaque, buf, len)。
3. p2p_peer_run 改为基于会话的便捷封装（去重）。
4. 自测：新增/改造 REGISTER_TEST_FUNC：session_open 后两线程互发 N 包后 close 通过。
5. VPN 侧按上节接入。

## 7. 边界

- 单会话=点对点；多对端/网格后续再加（届时会话可支持 peer 集合与包头）。
- 收包回调返回非 0 语义暂不特殊处理（预留）。
- TURN 需要时返回 -2（预留决策点）。
