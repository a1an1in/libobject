# STUN / UDP P2P 模块使用说明

基于 RFC 5389 STUN 的 **UDP NAT 穿透（打洞）**：处于不同 NAT 之后的两个节点各自上线后，
由发起方主动建链，最终点对点直连收发业务数据。中心服务器 `P2p_Server` 兼任
**信令（地址簿 + 撮合） + STUN 回显**，预留 TURN 中继（同进程）。

模型一句话：**p2p 节点 = 一个 Stun 节点（stun_id 上线）；p2p 会话 = 一条到某 remote
stun id 的链路**，每条会话独立 UDP data socket，打洞地址经信令交换。

设计/协议/接口细节见 [`p2p_design.md`](p2p_design.md)。

## 文件与职责

| 文件 | 作用 |
| --- | --- |
| [`src/include/libobject/net/p2p/p2p.h`](../../src/include/libobject/net/p2p/p2p.h) / [`src/net/p2p/p2p.c`](../../src/net/p2p/p2p.c) | **对外接口**（薄层）：`p2p_node_*`（节点生命周期）+ `p2p_session_*`（一条链路）+ `p2p_server_run` |
| [`src/net/p2p/P2p_Server.c`](../../src/net/p2p/P2p_Server.c) | 中心服务器：`SIGNIN/SIGNOUT` 登记 + `CALL/INVITE/ACCEPT/MATCH/PUNCHOK/CONNECTED` 撮合 + STUN 回显（预留 TURN） |
| [`src/net/p2p/stun/Stun.c`](../../src/net/p2p/stun/Stun.c) / [`Stun.h`](../../src/net/p2p/stun/Stun.h) | STUN 打洞客户端（内部）：节点 + 多会话管理，采址/打洞/保活/收发 |
| [`src/net/p2p/stun/Request.c`](../../src/net/p2p/stun/Request.c) / `Response.c` | STUN 请求/响应编解码（TLV、XOR-MAPPED-ADDRESS） |
| [`tests/net/test_p2p.c`](../../tests/net/test_p2p.c) | 测试命令：`test_p2p_server` / `test_p2p_peer`，用例 `test_p2p_loopback` / `test_p2p_multi` |
| [`src/net/turn/`](../../src/net/turn/) | TURN 中继（独立模块，预留接入） |

### 工作原理（简述）

1. 节点 `SIGNIN <stun_id>`：服务器记下它的**信令源地址**（后续 `INVITE`/`CONNECTED` 据此投递）。
2. 每条会话用**自己的 UDP socket**（绑 `local_host` + `local_service`/随机口）向公共 STUN 发
   Binding，取得**本会话的公网映射地址**（own）；配置了第二 STUN 则再采一次，比较外部端口
   判断是否对称（写 `nat_type`）。
3. 采址就绪后按角色把**本会话地址**带进信令：主叫 `CALL`，被叫 `ACCEPT`；服务器撮合回执
   `INVITE`/`MATCH` 把对方会话地址（含 `nat`）带回来。
4. 双方互发 `KEEPALIVE` 打洞（在各自 NAT 上为对方地址放行），收到对端任一 P2P 包即
   `PUNCHOK` 上报；服务器收齐双方上报才回 `CONNECTED`，此后 `is_connected` 为 0、可发业务数据。
5. 链路期周期 `KEEPALIVE` 维持 NAT 映射；**收到对端包时以真实源地址更新打洞目标**
   （源地址学习，见设计文档），以应对 NAT 对不同目的地分配不同端口。

> 关键机制：**采址、打洞、保活、业务数据必须走同一个会话 socket**，否则 STUN 报告的公网端口
> 与打洞出的映射不一致，公告地址失效。

## 构建

```bash
./devops.sh build --platform=linux
# 或增量
make -C build/linux/x86_64 -j$(nproc)
```

## 测试命令

> mockery 命令的参数里 `argv[0]` 是命令名，真实参数从第 2 个开始（代码已按此解析）。
> 统一加 `--log-type=0` 让日志输出到控制台；日志级别可用 `0x1ffff`（全量）或 `0x16`。

### 1) 进程内自动回环（无需外部服务器）

```bash
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 -f test_p2p_loopback
```

### 2) 进程内多路（A 同时连 B、C）

```bash
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 -f test_p2p_multi
```

### 3) 三进程本机回环（信令=STUN 共用）

```bash
# 终端1：信令服务器（绑定 127.0.0.1:9000）
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 9000

# 终端2：peerB —— 被叫（不带 <peer_id>，常驻等被叫）
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
      test_p2p_peer peerB - 127.0.0.1 9000

# 终端3：peerA —— 主叫（带 <peer_id>）
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
      test_p2p_peer peerA - 127.0.0.1 9000 peerB
```

两端出现 `CONNECTED ... link up` 与 `received N bytes: hello ...` 即双向互通。
**这只是回环验证，不经过真实 NAT**，只证明协议/流程正确。

### 4) 真机跨 NAT 部署

**服务器**（公网 IP 主机，放行 UDP 端口；阻塞运行，Ctrl+C 停止）：

```bash
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 12345
ss -lunp | grep 12345        # 确认 UDP 0.0.0.0:12345 在听
```

**两个节点**（各自在 NAT 后；被叫不带 peer_id，主叫带）：

```bash
# 节点 B：被叫（常驻）
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
      test_p2p_peer peerB 12346 10.10.10.115 12345

# 节点 A：主叫
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
      test_p2p_peer peerA 19001 119.4.206.14 12345 peerB
```

缺省 STUN 为公共服务器 `stun.cloudflare.com:3478`（主）+ `stun.l.google.com:19302`
（第二，用于对称探测）；特殊环境可用末尾参数覆盖：

```bash
test_p2p_peer peerA 19001 119.4.206.14 12345 peerB \
               stun.cloudflare.com 3478 stun.l.google.com 19302
```

## 参数说明（`test_p2p_peer`）

```
test_p2p_peer <stun_id> <local_service> <signal_host> <signal_port>
              [<peer_id> [<stun_host> <stun_port> [<stun2_host> <stun2_port>]]]
```

- `<stun_id>`：本节点标识（自定义字符串；需与对方的 `<peer_id>` 互相指认）；
- `<local_service>`：本端**会话(data)口**固定端口，`-` = 随机（多会话/防冲突用）；
  指定固定口便于云主机安全组放行与验证，**多会话并发时应随机**；
- `<signal_host> <signal_port>`：信令服务器地址（必填）；
- `<peer_id>`：**带 = 主叫**（主动 `CALL`）；**不带 = 被叫**（常驻等被叫）；
- `<stun_host> <stun_port>`：可选，覆盖主 STUN（缺省 `stun.cloudflare.com:3478`）；
  留空/同机环境可用信令服务器自身做采址；
- `<stun2_host> <stun2_port>`：可选，第二 STUN（对称探测，缺省 `stun.l.google.com:19302`）；
  也可配成信令服务器自身（它兼 STUN 回显）；不配则不做 nat 探测。

> 支持先后启动：先启动的一方上线后等对端上线/被叫，对端随后启动即可撮合。

## 日志与成功判定

- 服务端：`server SIGNIN: <id> registered (src ...)`、`server CALL/ACCEPT: ... sess=...:... nat=N`、
  `server CONNECTED: both punched ok A<->B`、`server SIGNOUT: <id> offline`；
- 客户端信令：`<id> signal recv: MATCH <peer> <host> <port> <nat>`、
  `<id> received MATCH from <peer> (<host>:<port> nat=N)`、`<id> CONNECTED to <peer>, link up`；
- NAT 探测：`stun1 target <host>:<port> -> <ip>`、`<id> nat_type=<CONE|SYMMETRIC|OPEN|UNKNOWN> (...)`
  （括号内打印判断依据：两个 STUN 的目标地址与各自回包映射）；
- 打洞/收包：`<id> received KEEPALIVE from <peer> (<src_host>:<src_port>), sent PUNCHOK`；
- 源地址学习：`<id> peer src differs: signaled <host>:<port> -> actual <host>:<port>`
  （信令上报地址与对端包真实源不同，已改用真实源回发）；
- 业务数据：`received N bytes: hello from ...`。

**成功判定**：两端都出现 `CONNECTED ... link up` 且能收到 `received N bytes`。

## 常见问题排查

### 服务器无日志 / 节点 SIGNIN 超时 / UDP 抓不到包

- 确认用最新构建的 xtools，且服务器监听 `ss -lunp | grep <port>`（不要重复启动，否则 `bind error`）；
- 节点发的是 **UDP**（与 node/http 的 TCP 不同）：`sudo tcpdump -i any -n -vv udp port 12345`；
  抓到 TCP 抓不到 UDP ⇒ **UDP 未放行**（云安全组/防火墙常只放行 TCP）。

### WSL2 下外部 UDP 进不来

- WSL2 默认 NAT 不转发入站 UDP（`netsh portproxy` 与 localhost 自动转发**仅支持 TCP**）；
- 开 mirrored 网络：`%UserProfile%\.wslconfig` 加 `[wsl2] networkingMode=mirrored`，
  `wsl --shutdown` 后重进，确认 WSL 的 eth0 出现主机局域网 IP；
- 放行 Windows 防火墙入站 UDP：
  `New-NetFirewallRule -DisplayName "UDP <port> P2P" -Direction Inbound -Action Allow -Protocol UDP -LocalPort <port> -Profile Domain,Private,Public`；
- mirrored 下还需放行 Hyper-V 防火墙：
  `Set-NetFirewallHyperVVMSetting -Name '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}' -DefaultInboundAction Allow`；
- 若公网 IP 在路由器上，需同时放行 **UDP** 端口映射。

### 两个节点在同一 NAT 后

- 服务器也在同一内网时可用内网 IP 测通（仅验证流程）；
- 两节点在同一远端 NAT 后连公网服务器时，打洞目标是该 NAT 自己的 WAN 地址，依赖 NAT
  **hairpin**，多数路由器不支持 → 可能失败；真正验证穿透需把两端放到**两个不同 NAT/公网出口**。

### 对称型 NAT

- 只有**双方都对称**才必须 TURN；一方对称、一方非对称通常仍能打通（源地址学习可提升成功率）；
- 探测原理：同一会话 socket 向两个不同 STUN 采址，比较外部映射端口：相同 → 非对称（可打洞）；
  不同 → 对称型（需 TURN）。类型值：`0=UNKNOWN 1=OPEN 2=CONE(非对称) 3=SYMMETRIC`；
- 需中继时借助 [`src/net/turn/`](../../src/net/turn/)（预留）。

## 使用免费公共 STUN（拆分 STUN 与信令）

自建服务器若与某个节点在同一 NAT 后，该节点无法用它发现自己的公网地址（hairpin 限制 /
服务器只能看到内网源）。因此默认把 **采址指向公网 STUN**，中心服务器只做信令 + 预留 TURN：

| 主机 | 端口 | 备注 |
| --- | --- | --- |
| `stun.cloudflare.com` | 3478 | 推荐（推荐主 STUN，anycast 稳定） |
| `stun.l.google.com` / `stun1.l.google.com` | 19302 / 3478 | 推荐第二 STUN（**须与主 STUN 不同 IP**） |
| `global.stun.twilio.com` | 3478 | 备用 |
| `stun.services.mozilla.com` | 3478 | 已失效（域名解析失败），勿用 |

### 约束

- 采址、打洞、保活必须是**同一个会话 socket**；
- 对称探测需要两个**不同目的**的 STUN；只配一个时 `nat_type` 保持 UNKNOWN（默认仍尝试打洞）；
- 需持续保活，避免 NAT 映射被空闲回收。

## 测试相关代码位置

- 用例与命令：[`tests/net/test_p2p.c`](../../tests/net/test_p2p.c)（节点经对外接口建立多会话）
- 对外接口：[`p2p.h`](../../src/include/libobject/net/p2p/p2p.h) + [`p2p.c`](../../src/net/p2p/p2p.c)
- 中心服务器：[`P2p_Server.c`](../../src/net/p2p/P2p_Server.c)
- STUN 打洞客户端（内部）：[`Stun.c`](../../src/net/p2p/stun/Stun.c)
- TURN 中继（预留接入）：[`src/net/turn/`](../../src/net/turn/)
