# STUN / UDP P2P 模块使用说明

本模块实现基于 RFC 5389 STUN 的 **UDP NAT 穿透（打洞）**，让处于不同 NAT 之后的两个 peer 能点对点直连。中心服务器（`P2p_Server`）兼任 **信令/地址簿 + STUN 回显** 角色，未来 TURN 中继也将并入同一进程。

## 架构与文件

| 文件 | 作用 |
| --- | --- |
| [`src/net/p2p/P2p_Server.c`](../../src/net/p2p/P2p_Server.c) | 中心服务器：信令 `REG/GET`(含 nat_type) + STUN 回显（预留 TURN 中继，同一进程） |
| [`src/include/libobject/net/p2p/p2p.h`](../../src/include/libobject/net/p2p/p2p.h) / [`src/net/p2p/p2p.c`](../../src/net/p2p/p2p.c) | **对外统一入口** `p2p_peer_run(cfg)`：探测 NAT + 信令交换 + 决策（打洞 or TURN），内部走打洞客户端 |
| [`src/net/p2p/stun/Stun.c`](../../src/net/p2p/stun/Stun.c) | STUN 打洞客户端（内部实现，仅 STUN 协议）：`discovery/probe/register_addr/lookup_addr/punch/send` |
| [`src/net/p2p/stun/Request.c`](../../src/net/p2p/stun/Request.c) / `Response.c` | STUN 请求/响应编解码（TLV、XOR-MAPPED-ADDRESS） |
| [`src/net/turn/`](../../src/net/turn/) | TURN 中继（独立模块，预留接入中心服务器） |
| [`tests/net/test_p2p.c`](../../tests/net/test_p2p.c) | 测试命令：`test_p2p_server` / `test_p2p_peer` |

### 工作原理

1. peer 用**同一个 UDP socket** 连接中心服务器；
2. `discovery`：向服务器发 STUN Binding，获得自己经 NAT 后的**公网映射地址**；
3. `register_addr(id)`：把自己注册到服务器（服务器记录"它看到的源地址"）；
4. `lookup_addr(peer_id)`：查询对端公网地址；
5. `punch`：直接向对端公网地址发打洞包，建立 NAT 映射；
6. `send`：互发业务数据；`keepalive`：周期保活维持映射。

> 关键机制：服务器登记/分发的对端地址 = **服务器观测到的报文源地址**。因此每个 peer 必须**通过自己的 NAT** 去连服务器，让服务器记下其公网映射，对端才打得到它。

## 打洞完整流程（UDP Hole Punching）

### 拓扑前提
- 一个**公网可达的 STUN/信令服务器**（地址簿 + 地址回显），两个 peer 都能连到它；
- peer1、peer2 各自在**不同的 NAT / 不同公网出口**之后（若在同一 NAT 后，见下方"同 NAT"限制）；
- 每个 peer 自始至终使用**同一个 UDP socket**（本地端口固定），保证 NAT 映射端口一致。

### 地址与标识的关系
- **peer id 只是字符串标识**（调用方指定），不含 ip:端口；
- 服务器以 id 为键登记该 peer **当前观测到的源地址（公网 ip:端口）**，即建立 `id ↔ 外网映射地址` 的动态关联；
- 对端只要给出对方的 id，就能从服务器查到对方的公网 ip:端口。

### 逐步时序（假设 peer1=内网 NAT-A 后，peer2=内网 NAT-B 后，服务器=公网）

1. **服务器启动**：`test_p2p_server <port>`，在公网 `0.0.0.0:<port>` 监听 UDP。

2. **peer1 上线**：本地绑定 `19001`，`connect` 中心服务器。
   - `discovery`：发 STUN Binding → 服务器回 XOR-MAPPED-ADDRESS；
   - peer1 的 NAT-A 为这条出向会话建立映射 `公网A:p1 ↔ 服务器`，peer1 得知自己的映射 `公网A:p1`。

3. **peer2 上线**：同样操作，NAT-B 建立映射 `公网B:p2`，peer2 得知 `公网B:p2`。

4. **注册**：
   - peer1 发 `REG peer1` → 服务器记录 `peer1 ↔ 公网A:p1`（源地址）；
   - peer2 发 `REG peer2` → 服务器记录 `peer2 ↔ 公网B:p2`。

5. **互查**：
   - peer1 发 `GET peer2` → 服务器回 `PEER 公网B p2`；
   - peer2 发 `GET peer1` → 服务器回 `PEER 公网A p1`。
   （若对端尚未注册，服务器回 `NOPEER`，此时查的一方**带重试等待**对端上线。）

6. **打洞（核心，必须双方都打）**：
   - peer1 把 UDP socket `connect` 切到 `公网B:p2`，发打洞包 `PUNCH`；
   - peer2 把 UDP socket `connect` 切到 `公网A:p1`，发打洞包 `PUNCH`；
   - 原理：peer1 主动向 `公网B:p2` 发包后，NAT-A 的映射 `公网A:p1` 就"认识"了 peer2 的源（公网B），从而**允许来自 公网B:p2 的回包**进入 peer1；peer2 对 NAT-B 同理。于是两个 NAT 各自放行了对方 → 直连通道建立。
   - **为什么必须双方都打**：若只有 peer1 打，peer1 的 NAT-A 认识 peer2 了，但 peer2 的 NAT-B 不认识 peer1 的源，peer2 发给 peer1 的包仍会被 NAT-B 丢弃 → 单向。

7. **互发业务数据**：双方直接向对方映射地址发 `DATA`，均能收到 → `P2P OK`。

8. **保活**：`keepalive` 每 ~1s 向对端发 `KEEPALIVE`，维持双方 NAT 映射不因空闲而超时回收。

### 判定与日志
- NAT 探测（给了第二 STUN 时）：打印 `NAT probe: external port stable/...` 或 `SYMMETRIC`，`[<id>] stun_peer_run mapped: ... nat_type=N`；
- 注册：`[<id>] registered public addr 公网X:端口 nat_type=N to signaling`；服务器侧 `[server] REG id=... record ... nat=...`；
- 查到对端：`[<id>] peer <peer_id> address: 公网Y:端口, nat_type=N`（N 为对端上报类型）；`punch -> 公网Y:端口`；
- 收到对端数据：`[<id>] stun_peer_run: received peer data`，业务回调 `[<id>] recv N bytes: hello from ...`；
- 双对称（本端与对端都 SYMMETRIC）：不打洞，`stun_peer_run` 返回 `-2`（需 TURN），日志 `need TURN`；
- 服务器日志对应 `[server] REG id=... / [server] GET id=... / [server] recv ... from ...`。

### 常见失败点（对应排障）
- **register/lookup 超时**：peer 到服务器 UDP 不通（服务器没起 / 只放行了 TCP / 端口被占），先解决"peer→服务器 UDP 可达"；
- **peer 登记成私网/127.0.0.1**：peer 连服务器走了内网/回环路径，导致查到的地址对端打不到；
- **两个 peer 在同一 NAT 后**：打洞目标是该 NAT 自己的 WAN 端口，需 NAT **hairpin** 支持，多数不支持 → 失败；应把两端放到两个不同公网出口；
- **对称型 NAT**：打洞目标（每次新目的地址）会重新分配映射端口，advertised 端口失效 → 失败，需 TURN 中继。

## 构建

```bash
./devops.sh build --platform=linux
```

## 测试命令

> mockery 命令的参数里 `argv[0]` 是命令名，真实参数从第 2 个开始（代码已按此解析）。

统一加 `--log-type=0` 让日志输出到控制台，日志级别可用 `0x1ffff`（全量）或 `0x16`。

### 1) 本机回环验证（无需任何外部服务器；起 3 个子进程/终端，信令=STUN 共用）

```bash
# 终端1：信令服务器（绑定 127.0.0.1:9000）
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 9000

# 终端2：peerA（本地打洞口 19001）
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
      test_p2p_peer peerA 19001 127.0.0.1 9000 peerB 127.0.0.1 9000

# 终端3：peerB（本地打洞口 19002）
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
      test_p2p_peer peerB 19002 127.0.0.1 9000 peerA 127.0.0.1 9000
```

两个 peer 日志都出现 `stun_peer_run: received peer data` 即双向互通、验证通过。注意：**这只是回环验证，不经过真实 NAT**，只能证明协议/流程正确，不能证明穿透能力。

### 2) 真机跨 NAT 部署：一台公网 STUN 服务器 + 两个 peer

**服务器**（公网 IP 主机，需放行 UDP `<server_port>`；阻塞运行，Ctrl+C 停止）：

```bash
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 12345
```

启动后确认监听（不要重复启动，否则会 `bind error`）：

```bash
ss -lunp | grep 12345   # 应看到 UDP 0.0.0.0:12345
```

**peer1 / peer2**（两台各自在 NAT 后的机器；local_port 可为各自本地端口，不同机器可相同）：

```bash
# 机器 A（带第二 STUN，自动探测 NAT 对称性）
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
      test_p2p_peer peer1 19001 119.4.206.14 12345 peer2 stun.cloudflare.com 3478 stun1.l.google.com 3478

# 机器 B
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
      test_p2p_peer peer2 12346 10.10.10.115 12345 peer1 stun.cloudflare.com 3478 stun1.l.google.com 3478
```

- 参数后两段 `<stun2_host> <stun2_port>` 为**可选**的第二个公共 STUN：给了它 `stun_peer_run` 会自动 `probe`（两次采址比较外部端口）并把本端类型经 `REG` 上报、从 `PEER` 拿到对端类型；
- `peer1/peer2` 是自定义标识，只需保证两台命令里"我的 id"与"对方的 peer_id"互相指认；
- **支持先后启动**：先启动的一方会带着重试等待对端上线（约 60 秒窗口），对端随后注册即可被查到；
- 也可同时启动（更稳）。

**成功判定**：两台 peer 日志都出现 `[<id>] stun_peer_run: received peer data`（表示收到对端数据）且服务器日志出现 `[server] REG id=... / [server] GET id=...`；若两端都对称，`stun_peer_run` 返回 `-2`（需 TURN），日志提示 `need TURN`。

### 3) NAT 对称性探测（已并入 `stun_peer_run`，无单独测试命令）

- 探测原理 = 用**同一个 UDP socket** 向两个不同 STUN 采址，比较两次外部映射端口：相同 → 非对称（可打洞）；不同 → 对称型（需 TURN）；
- 公开方法 `Stun.probe()`（两次采址比较，写 `stun->nat_type`，返回 1/0/-1）；在 `test_p2p_peer` 末尾加第二个 STUN 参数即由 `stun_peer_run` 自动完成探测；
- 判定只需区分是否对称：**任一端非对称即可打洞；两端都对称 → 打洞必败，返回 `-2`，需 TURN 中继**；
- 类型值：`0=UNKNOWN 1=OPEN 2=CONE(非对称) 3=SYMMETRIC`。

## 常见问题排查

### 服务器无日志 / peer register 超时 / UDP 抓不到包
- STUN 服务器旧版本收包时**不打印日志**（已在新版加 `[server] recv/REG/GET`），请用最新构建的 xtools 并确认监听 `ss -lunp | grep <port>`；
- peer 发的是 **UDP**，与 node/http 的 **TCP** 不同。抓包判定：`sudo tcpdump -i any -n -vv udp port 12345`；
  - 抓到 TCP 抓不到 UDP ⇒ UDP 未放行（云安全组/防火墙按协议放行，常只放了 TCP）；
- 服务器若显示 `service:0` / `bind error` ⇒ 参数解析或端口被占用问题（确保只起一个服务器、命令带端口参数）。

### WSL2 下外部 UDP 进不来
- WSL2 默认 NAT 不转发入站 UDP（`netsh portproxy` 与 localhost 自动转发**仅支持 TCP**）；
- 开启 mirrored 网络：Windows `%UserProfile%\.wslconfig` 加 `[wsl2] networkingMode=mirrored`，`wsl --shutdown` 后重进，并确认 WSL 的 eth0 出现主机局域网 IP；
- 放行 Windows 防火墙入站 UDP：`New-NetFirewallRule -DisplayName "UDP <port> P2P" -Direction Inbound -Action Allow -Protocol UDP -LocalPort <port> -Profile Domain,Private,Public`；
- mirrored 下仍需放行 Hyper-V 防火墙：`Set-NetFirewallHyperVVMSetting -Name '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}' -DefaultInboundAction Allow`；
- 若公网 IP 在路由器上做端口映射，需同时放行 **UDP** 映射。

### 两个 peer 都放一个内网/同一 NAT 后
- 服务器也在同一内网时可用内网 IP 测通（仅验证流程）；
- 两个 peer 在**同一远端 NAT** 后连公网服务器时，peer 之间打洞目标是该 NAT 自己的 WAN IP，依赖 NAT **hairpin**，多数路由器不支持，可能失败；
- 真正验证穿透需把两个 peer 放到**两个不同 NAT / 不同公网出口**之后。

### 对称型 NAT
- 只有**双方都对称**才必须 TURN（`stun_peer_run` 返回 `-2`）；一方对称、一方非对称通常仍能打通；
- 需转 TURN 时借助 `turn/`（TURN 中继）模块做数据中转。

## 使用免费公共 STUN（拆分 STUN 与信令）

若不想/不能自建公网 STUN（例如和信令同 NAT 的 peer 需要拿到真实公网地址），可把 **discovery 指向免费公共 STUN**，信令服务器只做地址交换。

> **为什么目前只把自建 P2p_Server 当“信令”用、而 discovery 走公共 STUN（不使用其 STUN 回显）？**
> 因为存在 **hairpin 问题**：若 P2p_Server 与某个 peer 在同一个 NAT 之后——
> - peer 访问服务器的**外网映射地址**时，需要该 NAT 支持 hairpin 才能回环到内网服务器（多数不支持，peer 连不上）；
> - 而 peer 若访问服务器的**内网地址**，服务器只会看到 peer 的内网源，**取不到 peer 自己的外网地址**。
> 所以同 NAT 的 peer 无法用“与它同 NAT 的服务器”发现自己的公网地址；只有让 discovery 指向一个**真正在公网、peer 能直接外拨到达**的 STUN（如免费公共 STUN），peer 才能拿到真实公网映射。中心服务器于是只承担 **REG/GET 地址交换（信令）+ 预留 TURN 中继**。

**可用（已从本机实测能正确回 STUN Binding，魔数 0x2112a442）：**
| 主机 | 端口 | 备注 |
| --- | --- | --- |
| `stun.cloudflare.com` | 3478 | 推荐（Cloudflare 维护，anycast，稳定） |
| `stun1.l.google.com` / `stun2.l.google.com` | 3478 | 实测可用（曾用于 19302） |
| `global.stun.twilio.com` | 3478 | 可用 |
| `stun.services.mozilla.com` | 3478 | 已失效（域名解析失败），勿用 |

**推荐默认**：`stun.cloudflare.com:3478`；可配一组做容灾（Cloudflare → Google stun1 → Twilio）。

### 拆分后的流程
1. Peer1/Peer2 各自用**同一个 UDP socket** 向公共 STUN 发 Binding → 得到各自 NAT 的公网映射地址；
2. （可选 NAT 探测）再向**另一个公共 STUN** 采址，比较外部端口 → 判断本端是否对称（写 `nat_type`，两个 STUN 建议 `stun.cloudflare.com` + `stun1.l.google.com`）；
3. 各自把公网地址**与 nat_type** 报到自己的信令服务器：`REG <id> <host> <port> <nat_type>`（服务器可放任意双方可达处，与 peer 同 NAT 也没关系）；
4. 信令服务器把 Peer1 地址/类型给 Peer2、Peer2 地址/类型给 Peer1：`PEER <host> <port> <nat_type>`（按 id/session 交换）；
5. 双方判定：**任一端非对称 → 直接打洞**（跨不同 NAT，不需要 hairpin）；**两端都对称 → 打洞必败，对外 `p2p_peer_run` 返回 `-2`，转 TURN 中继（预留）**。

### 约束
- discovery/probe 与打洞必须是**同一个 socket**（否则 STUN 报告的公网端口与打洞用的映射不一致，公告地址失效）；
- 对称探测需要两个**不同目的**的 STUN，仅一个 STUN 时无法区分对称型（此时类型为 UNKNOWN，默认尝试打洞）；
- 需持续保活，维持 NAT 映射端口不回收。

## 测试相关代码位置
- 测试用例与命令：`tests/net/test_p2p.c`（peer 经对外统一入口 `p2p_peer_run` 建立会话）
- 对外建链接口：`src/include/libobject/net/p2p/p2p.h` + `src/net/p2p/p2p.c`
- 中心服务器（信令+STUN 回显，预留 TURN）：`src/net/p2p/P2p_Server.c`
- STUN 打洞客户端（内部实现/原语）：`src/net/p2p/stun/Stun.c`
- TURN 中继实现（预留接入）：`src/net/turn/`
