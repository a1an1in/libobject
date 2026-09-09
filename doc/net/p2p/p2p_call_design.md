# p2p 在线节点 + 服务器确认式打洞（对称，无 listen，回调分层）

> 目标：任一端随时可发起连接，另一端临时在线即可被叫。
> 模型：**两个 peer 都各自 `p2p_session_open`（上线 + 内置处理服务器/预打洞消息，
> 不阻塞）；要连别人时由发起方 `p2p_peer_connect`。连接“真正成功”以
> 服务器为准——双方各自打洞成功后上报，服务器收齐两边上报才判定两端 ok
> 并通知双方(CONNECTED)，此时 open/connect 那侧置 connected、可发业务数据。**
> ACCEPT 只表示“愿意配合打洞”，不等于连接成功。
> 状态：设计定稿（V3，2026-09），待实现。

## 1. 模型与回调分层（关键）

- 每个节点 `p2p_session_open`：构造 + 连信令 + 采址/探测 NAT + REG 上线 +
  启动**内置**的信令/预打洞处理。**不阻塞**；返回即本端在线(可被叫、可 connect)。
- **消息按来源/阶段分流（同一 UDP 端口收包，按内容区分回调）**：
  - **服务器信令消息**（REG OK / INVITE / PEER / NOPEER / CONNECTED）→
    内置处理，**不暴露给用户**；
  - **peer 的预打洞消息**（打洞握手/保活，链路建立前）→ 内置处理，
    **不暴露给用户**；
  - **peer 的打洞成功后消息**（业务数据）→ 唯一暴露给用户的 `recv` 回调
    （接口配置的只有它）。
- 内置信令处理职责：
  - 被叫：收到 INVITE → 自动回 ACCEPT（愿意配合打洞）→ 打洞对齐 → 上报服务器
    → 收到 CONNECTED → 置本端 connected；
  - 发起方 connect：CALL → 打洞对齐 → 上报服务器 → 收到 CONNECTED → 返回。
- 上线/下线：REG=在线；close 发 BYE 下线（不做心跳/TTL，崩溃残留为后续增强）。

## 2. 连接建立（服务器确认式，ACCEPT≠成功）

```
A(open)         S(P2p_Server)      B(open, 内置应答)
  |-- CALL A B -->|                  |
  |<-- WAIT ------|                  |
  |               |-- INVITE A <A_host> <A_port> -->|
  |               |<-- ACCEPT A --------------------|   ← 只是愿意配合打洞
  |<-- PEER B <B_host> <B_port> <B_nat> --|          |   ← 服务器把对方地址给双方
  |-- 打洞 --------> (B 收到 A 包则打洞成功)         |
  |               |<-- PUNCHOK A(打洞成功上报) ------|(B 也打洞到 A)
  |-- PUNCHOK B -->|                                 |   ← 双方都上报成功
  |<-- CONNECTED --|------------------ CONNECTED --> |   ← 服务器判定两端 ok
  A connect 返回 0 / 置 connected          B 内置置 connected
```

- 哪边成功收到对方 P2P 包（打洞通）→ 向服务器发一次 `PUNCHOK`；
- 服务器对一次呼叫收集**双方都 PUNCHOK** → 才发 `CONNECTED` 给两边；
- 只收到一方上报（单向通）→ 不判定成功，等超时失败。

## 3. 信令文本协议

- `REG <id> <host> <port> <nat>` → `OK`
- `BYE <id>` → 删除在线表项
- `GET <id>` → `PEER <host> <port> <nat>` | `NOPEER`（兼容/直连查表）
- 撮合：
  - A→S：`CALL <from> <to>`（S 记 pending、回 A `WAIT`、发 B `INVITE <from> <from_host> <from_port>`）
  - B→S：`ACCEPT <from>`（配合打洞）| `REJECT <from>`
  - S→A（ACCEPT 后）：`PEER <B_host> <B_port> <B_nat>`
  - A/B→S：`PUNCHOK <caller> <reporter>`? →（简化见 §3.1）
  - S→A/B（双方都 PUNCHOK）：`CONNECTED <peer_id>`

### 3.1 PUNCHOK 上报归属（避免遍历 pending）
服务器 pending 以发起方 id 为 key（一次 CALL 一条）。上报消息带两个 id：
`PUNCHOK <id1> <id2>`，含义“id1 已成功打洞到 id2”。服务器：
- 搜 pending[id1] 且 to==id2 → 发起方(id1) 上报 → 记 caller_ok；
- 否则搜 pending[id2] 且 to==id1 → 被叫方(id1) 上报 → 记 callee_ok；
双方都记满 → 发 CONNECTED 给 call 的两端并删 pending。
（并发/1:1 范围外多呼，见 §7。）

## 4. 公共接口定稿（p2p.h，最小可见面）

```
int p2p_session_open(p2p_session_t **out, p2p_recv_fn recv,
                     const p2p_cfg_t *cfg, void *opaque);
/* 0=已上线(在线，可被叫/可 connect)；-1=失败。不阻塞，内部信令/预打洞内置。 */

int p2p_peer_connect(p2p_session_t *s);
/* 发起方阻塞；0=打通(CONNECTED，服务器确认双方打洞成功，可 send)；
 * -1=对端不在线/失败；-2=双对称需 TURN；-3=超时。 */

int p2p_session_is_connected(p2p_session_t *s);
/* 0=已与某 peer 建立(可 send)；-1=未建立/已关闭。被叫侧据此轮询/等待。 */

int p2p_session_send(p2p_session_t *s, const uint8_t *data, int len); /* 需 connected */
int p2p_session_close(p2p_session_t *s);   /* 停内部处理+BYE 下线+释放 */
int p2p_session_is_alive(p2p_session_t *s);/* 0=在线 */
int p2p_server_run(const char *host, const char *service);
```

`recv` 只在**打洞成功后**被回调（业务数据）；打洞前的握手/预打洞消息内置消化，
不外发。

## 5. 实施步骤

1. P2p_Server：pending 加 caller_ok/callee_ok；CALL/ACCEPT/REJECT/INVITE 已有；
   ACCEPT → 发 A PEER；`PUNCHOK` → 记两方 ok，双 ok → 发两端 CONNECTED。
2. 客户端 p2p：内置信令处理 + 预打洞处理；open=构造+上线(+不阻塞)；
   connect=CALL→打洞→PUNCHOK→等 CONNECTED→返回；置 connected；保活。
3. Stun 层只需把“服务器文本行(INVITE/CONNECTED/PEER/PUNCH 接收事件)”上抛给
   p2p 内置层；业务 DATA 才走用户 recv（预打洞 PUNCH 与保活内部处理，不外发）。
4. 自测：A/B 各 open；A connect 成功→双向 hello；B close(BYE) 后 A connect →NOPEER。
5. 接入 VPN：常驻端 open（等待被叫）；另一端 open+connect。

## 6. 语义边界

- 双向都通才成功：单向通（一方 NAT 异常）不会误报成功。
- 打洞前/后消息以“是否收到 CONNECTED / connected”分界：建立前 peer 的
  PUNCH/保活内置，建立后 DATA 才交给用户 recv。
- 双对称 NAT：双方上报永远不齐 → 超时 -3（可再由 nat 预判快速 -2，后续）。

## 7. 边界/后续

- 并发多呼、鉴权/白名单、防重放、心跳 TTL、TURN 中继、在线订阅感知。
