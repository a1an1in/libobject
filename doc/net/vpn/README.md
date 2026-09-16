# VPN 模块使用说明（net/vpn）

基于 [`p2p`](../p2p/README.md) 穿透通道的上层 **站点间网段互访（L3 VPN）**：
每个节点建一个 Linux TUN 虚拟网卡，本机发往对端网段的 IP 包经 p2p 直连通道透传，
对端收到后写入自己的 TUN 注入协议栈，从而 `ping` 通对端隧道地址 / 对端内网主机。

设计与取舍见 [`p2p_vpn_design.md`](p2p_vpn_design.md)。

## 文件与职责

| 文件 | 作用 |
| --- | --- |
| [`src/include/libobject/net/vpn/vpn.h`](../../src/include/libobject/net/vpn/vpn.h) | **对外接口**：`vpn_cfg_t` + [`vpn_run()`](../../src/include/libobject/net/vpn/vpn.h) |
| [`src/net/vpn/Vpn_Internal.h`](../../src/net/vpn/Vpn_Internal.h) | **模块内部定义**：常量、控制帧协议（`vpn_ctrl_t`）、`vpn_net_t`/`vpn_link_t`/`vpn_ctx_t`、结果码 |
| [`src/net/vpn/Vpn.c`](../../src/net/vpn/Vpn.c) | 入口实现：建链 → 交换地址 → 配 tun → 双向转发（多对端选路） |
| [`src/net/vpn/Vpn_Command.c`](../../src/net/vpn/Vpn_Command.c) | **命令行** `xtools vpn ...`（解析选项 → 组装 `vpn_cfg_t` → `vpn_run`） |
| [`src/net/vpn/Vpn_Server_Command.c`](../../src/net/vpn/Vpn_Server_Command.c) | **服务端命令行** `xtools vpnserver ...`（信令+STUN 服务器，复用 `p2p_server_run`） |
| [`src/net/vpn/tun/Tun.h`](../../src/net/vpn/tun/Tun.h) | Tun 抽象（vpn 内部；`open/configure/route_add/read/write/set_mtu/close`） |
| [`src/net/vpn/tun/os/unix/Tun.c`](../../src/net/vpn/tun/os/unix/Tun.c) | Linux 实现（`/dev/net/tun`，L3 `IFF_TUN\|IFF_NO_PI`） |
| [`tests/net/test_vpn.c`](../../tests/net/test_vpn.c) | 测试命令：`test_vpn_tun`（Tun 自检）/ `test_vpn_peer`（端点，复用 `Vpn_Command`） |

分层：`src/net/vpn` 与 `src/net/p2p` **平级并列**，`vpn -> p2p` 单向依赖；VPN 只用 p2p 公共头。
Tun **内聚在 vpn 内部**，等出现第二使用者再上提为公共抽象层（见设计文档 §8）。

## 能力范围

- Linux TUN、**L3 路由模式**；配置地址/路由走**外部 `ip` 命令**，MTU 走 `ioctl`；
- **一个 tun 复用给多条链路（多对端）**：被动方（被连的一方）可同时接多个对端（上限 8），
  出站按目的地址选链路（对端隧道地址精确 / 对端内网网段最长前缀）；
- **隧道地址由被动方分配**：主动方可以完全不带隧道地址，由对端在应答里分配；
- **对端内网网段只来自运行时交换**（无静态路由配置入口）；
- **不加隧道头**（一个对端一个会话 socket，身份由 socket 决定）、**不加加密**；
- 保活由 p2p 会话内部维持（VPN 不感知）。

不做（后续增强）：对端网段冲突仲裁、跨对端中转（A↔C 经 hub）、TAP/二层 + ARP、
Windows TAP、加密、TURN 中继。

## 权限要求

打开/配置 TUN（`TUNSETIFF`、`SIOCSIFMTU`、`ip addr/route`）需要 **root 或 CAP_NET_ADMIN**。
普通用户运行会看到：

```
tun: TUNSETIFF on (auto) failed: Operation not permitted
vpn: tun open failed (need root/CAP_NET_ADMIN?)
```

## 构建

```bash
./devops.sh build --platform=linux
# 或增量（新增 .c 文件后需先重跑一次 cmake 以刷新 GLOB；只加 .h 不需要）
cmake -S . -B build/linux/x86_64 -DPLATFORM=linux && make -C build/linux/x86_64 -j$(nproc)
```

## 命令行（正式入口）

### 服务端：`xtools vpnserver`

VPN 没有独立服务端，它复用 p2p 的中心服务器（同一 UDP 端口兼**信令地址簿/撮合 + STUN 回显**，
预留 TURN）：

```bash
# 监听 0.0.0.0:12345（UDP），阻塞运行，Ctrl+C 停止
# （端口 >1024，普通用户即可；只有 vpn 节点需要 root）
./sysroot/linux/x86_64/bin/xtools vpnserver -l 0.0.0.0:12345
ss -lunp | grep 12345        # 确认 UDP 在听
```

| 选项 | 默认值 | 说明 |
| --- | --- | --- |
| `-l, --listen <host:port>` | `0.0.0.0:12345` | 监听地址（UDP）；`:12345` 等价 `0.0.0.0:12345` |

### 节点：`xtools vpn`

**被动方（被连的一方）= hub，同时也是隧道地址的分配者**：它必须有自己的隧道地址，
并把它所在网段当作地址池（建议用网段内的大地址，如 `.254`）。**主动方可以不填隧道地址**。

```bash
# 节点 B（被动方/hub）：隧道地址 10.0.0.254/24（= 地址池），自己的内网 10.10.10.0/24
sudo ./sysroot/linux/x86_64/bin/xtools vpn \
     -i vpnB -l 12346 -s <服务器>:12345 \
     --tunnel-ip 10.0.0.254/24 --local-net 10.10.10.0/24

# 节点 A（主动方）：不填 --tunnel-ip，地址由 B 分配；只填自己的内网 172.16.10.0/23
sudo ./sysroot/linux/x86_64/bin/xtools vpn \
     -i vpnA -p vpnB -l 19001 -s <服务器>:12345 \
     --local-net 172.16.10.0/23
```

| 选项 | 必填 | 默认 | 说明 |
| --- | --- | --- | --- |
| `-i, --id <stun_id>` | ✅ | — | 本端 stun id |
| `-p, --peer <stun_id>` | | 空 | 对端 stun id；**省略=被动方**（常驻等被连） |
| `-s, --signal <host:port>` | ✅ | — | 信令服务器；**必须带端口** |
| `--tunnel-ip <a.b.c.d[/len]>` | **被动方必填** | — | 本端隧道地址。被动方建议 `x.x.x.254/24`：**它同时是地址池**（对端依次分到 `.1/.2/...`）；**主动方可省**（由对端分配） |
| `--local-net <本端网段>` | | 空=不通告 | **只填自己的内网网段**（如 `172.16.10.0/23`）；链路建立后自动交换，对端自动加路由 |
| `--stun <host[:port]>` | | `stun.cloudflare.com:3478` | 采址用 STUN；`none`/`off`/`0`=禁用（回退到**用信令服采址**，同机回环常用） |
| `--stun2 <host[:port]>` | | `stun.l.google.com:19302` | 第二个 STUN（判断 NAT 是否对称）；`none`/`off`/`0`=禁用 |
| `-l, --local-service <port>` | | 空=随机 | 本端数据口(UDP)；**防火墙只放行特定端口时必须固定它** |
| `-t, --tun <name>` | | 空=自动 | tun 设备名（自动时内核分配 `tunN`） |
| `--interval <ms>` | | `200` | 打洞/保活周期 |

> 被动方**必须**有自己的隧道地址（它既是自身地址、也是分配池）；漏填会在启动时直接报错。

### 地址交换：一个来回拿到双方地址（先交换、后配 tun）

```
主动方 A                                      被动方 B（hub = 分配者）
  │── NET_NOTIFY    payload = "A 的内网网段"（可空） ──►│
  │                                        B: 从地址池分配一个隧道地址给 A
  │◄── NET_NOTIFY_ACK payload = "0 <分给A的地址/len> <B 的隧道地址/len>[ <B 的内网网段>]" ──│
  │   A: 记下自己的地址 + B 的地址/网段        B: 记下 A 的地址/网段
  └────────── 双方各自配 tun（本端地址 + 对端网段路由），随后开始转发 ──────────┘
```

关键点：

| 项 | 说明 |
| --- | --- |
| 谁发通告 | **只有主动建链方**发一次 `NET_NOTIFY`（未收到应答时约 1s 重发，最多 10 次，之后报超时） |
| 谁分配地址 | **被动方**（被连的一方）。池 = 它的 `--tunnel-ip` 所在网段，占用表管理 |
| 地址回收 | 被动方在转发循环里巡检：连续 5 轮不可用判定断链 → 关会话 + **归还地址** + 空出槽位；**只回收被动链路**（主动方那条保留，留 p2p 自愈余地） |
| 何时配 tun | 地址齐备之后**一次性**配好（本端地址 + 各对端网段路由），不会"先配错再改" |
| 无 `--local-net` | 通告 payload 为空：不交换网段，只能 `ping` 通对端**隧道地址** |
| 网段校验 | 对端网段必须是合法 `a.b.c.d/len`（`/1`~`/32`，拒绝 `0.0.0.0/x` 与 `/0`）；非法则回拒绝码并告警 |

### 多对端：一个 hub 接多个 peer（单 tun 复用）

**模型**：一个 tun 复用给多条链路（link）。内核路由只负责"把包**引进** tun"，
"发给**哪个**对端"由 vpn 层按目的地址决定（TUN 是无 ARP 设备，内核给不出 per-peer 下一跳）：

| 目的地址 | 选哪条链路 |
| --- | --- |
| 某对端的**隧道地址**（如 `10.0.0.1`） | 精确命中该对端 |
| 某对端的**内网网段**（如 `172.16.10.0/23`） | 最长前缀匹配 |
| 都不命中、且只有一条链路 | 就用它（点对点兼容退化） |
| 都不命中、且有多条链路 | **丢弃**（不猜，避免串给别的对端），日志 `出站包无匹配的对端` |

```bash
# B（被动方/hub）：只跑一个进程、一个 tun，不需要预配置"谁会来连我"
sudo xtools vpn -i vpnB -s <server>:12345 --tunnel-ip 10.0.0.254/24 --local-net 10.10.10.0/24

# A、C（主动方）：各自连 B，**不填隧道地址**（由 B 分配），只填自己的内网网段
sudo xtools vpn -i vpnA -p vpnB -s <server>:12345 --local-net 172.16.10.0/23
sudo xtools vpn -i vpnC -p vpnB -s <server>:12345 --local-net 192.168.5.0/24
```

B 的日志会依次出现：

```
vpn: 新增对端链路 slot=0（在线 1 条，被动）
vpn: 分配隧道地址 10.0.0.1 给新对端
vpn: 已接受对端通告，分配 10.0.0.1/24 给对端（本端 10.0.0.254/24）
```

A/C 侧：

```
vpn: 已向对端通告本端内网 172.16.10.0/23，等对端分配地址
vpn: 对端已接受本端通告，分配本端隧道地址 10.0.0.1/24
vpn: tun tunX up, tunnel-ip=10.0.0.1/24 mtu=1400
```

| 注意 | 说明 |
| --- | --- |
| 共享隧道段 | 各对端隧道地址都在 `10.0.0.0/24` 里 → 本机直连路由包含它们；A ping C 的隧道地址会被 B **二次转发**给 C（B 按"对端隧道地址"选路）。想彻底避免借道，就给每对端一条独立隧道段 |
| A↔C 的内网互访 | 当前**只保证"每个对端 ↔ hub"**：B 不把 A 的网段再通告给 C，也不做跨对端中转（需要 B 开 `ip_forward` 并显式配路由） |
| 对端网段重叠 | 本版**不检测**冲突：两个对端通告同一网段时按最长前缀 + 先到者优先 |
| 被动方上限 | 同时在线对端最多 `VPN_MAX_LINKS`（8）；地址池容量相同，满了回 `VPN_NOTIFY_ENOADDR` |

### 端口与防火墙

| 端口 | 是谁 | 放行 |
| --- | --- | --- |
| `<服务器>:12345`（UDP） | 信令 + STUN 服务器（所有节点共用） | 服务器侧放行**入向** UDP 12345 |
| `-l` 指定的**本端数据口**（UDP） | 本端每个会话自己的 socket | 防火墙只放行特定端口时：固定它并放行**入向** |
| 出向 UDP | 本机 → 服务器 / 对端 | 一般默认放行；打洞链路靠出向建立、保活维持 |

- `-l` 省略 = 随机端口（NAT 后普通场景够用）；**只有"防火墙只放行特定端口"的机器才必须固定**。
- 启动日志会打印实际生效的端口：`vpn: id=vpnB peer=(callee) signal=... service=12346 ...`
- 被动方也要放行自己的 `-l` 端口；服务器端口只有一个，不需要每对端一个。
- 一个 `vpn` 进程**可以接多个对端共用一个 tun**（被动方）；主动方（带 `-p`）主动连一个对端。

### 排障：两端迟迟看不到 `connected`

按可能性排序：

1. **采址 STUN 不合适（最常见）**：若把 `--stun` 指向**与被连方同网段的信令服**，它会采到
   **内网地址**，对端拿这个地址去打洞必然打不通。默认已用公网 STUN；也可显式把两端
   `--stun` 都指到**公网可达**的 STUN。日志锚点：`nat_type=`、`received MATCH from ...`。
2. **UDP 未放行**：服务器侧入向 UDP `12345`；本机出向 UDP（`-l` 只管入向）。
   `sudo tcpdump -i any -n -vv udp port 12345` 确认有来包。
3. **id / peer 不匹配**：主动方用 `-p vpnB`；被动方必须 `-i vpnB` 且**不带 `-p`**。
4. **两端连的不是同一台信令服**：`-s` 的 host 可以是内网口或公网口，但必须是同一台。
5. **双对称 NAT**：两端都 `SYMMETRIC` 时打洞大概率失败（需 TURN，首版预留）。

已 `connected` 但链路建不起来，看这几条：

| 日志 | 原因 |
| --- | --- |
| `等待对端分配隧道地址超时(60s)` | 被动方没回应答：确认它是被动方、`--tunnel-ip` 合法、地址池未满 |
| `无法分配隧道地址（池未配置或已耗尽）` | 被动方 `--tunnel-ip` 未配/非法，或同时在线对端已达 8 |
| `出站包无匹配的对端` | 目的地址既不是对端隧道地址、也不在任何对端网段里（对端没配 `--local-net`？） |
| `链路连续 5 轮不可用，判定断链并回收` / `释放链路（归还隧道地址 x.x.x.x）` | 对端掉线，地址已归还（正常回收，不是错误） |

抓流程用日志锚点：`MATCH` → `KEEPALIVE(打洞)` → `PUNCHOK` → `CONNECTED`，缺哪一环就查哪一环。

`xtools vpn --help` 可看全部选项。**注意**：`vpn`/`vpnserver` 都是常驻命令
（阻塞到 Ctrl+C），请在独立终端运行。

## 测试与验证

> **三条路，按需挑一条**：
>
> | 方式 | 命令 | 覆盖范围 | 前置条件 |
> | --- | --- | --- | --- |
> | **命令行（推荐）** | `xtools vpnserver` + `xtools vpn` | 完整隧道（真机跨 NAT / 同机回环 / 多对端） | ≥1 台机器，需 root |
> | mockery 等价写法 | `test_p2p_server` + `test_vpn_peer` | 同上（**同一条实现路径**） | 同上 |
> | 单机自检 | `test_vpn_tun` | 只到 Tun 层（open/configure/route_add/read） | 单机 + root |
>
> - `test_vpn_peer` 与 `xtools vpn` **同实现、同默认值**：内部就是构造 `Vpn_Command`、跑同一个
>   `run_command`；区别只是它只能用**位置参数**（mockery 会把 `-` 开头的实参当自己的选项处理）。
> - `vpn` / `vpnserver` / `test_vpn_peer` 都是**常驻阻塞**命令（Ctrl+C 退出），请放在独立终端运行。
> - 下文示例地址：服务器公网 `119.4.206.14` / 内网口 `10.10.10.115`；A 侧内网 `172.16.10.33/23`；
>   B 侧内网 `10.10.10.115/24`；隧道段 `10.0.0.0/24`（B 用 `.254`，A/C 由 B 分配 `.1/.2`）。

### 1) 真机跨 NAT：命令行（推荐）

**① 服务端**（放行 UDP 12345）

```bash
./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 vpnserver -l 0.0.0.0:12345
ss -lunp | grep 12345        # 确认 UDP 在听；Ctrl+C 停止
```

**② 节点 B**（被动方/hub）：隧道地址用 `10.0.0.254/24`（= 地址池），`-s` 用 B 可达的服务器地址

```bash
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 vpn \
     -i vpnB -l 12346 -s 10.10.10.115:12345 \
     --tunnel-ip 10.0.2.1/24 \
     --local-net 10.10.10.0/24
```

**③ 节点 A**（主动方）：`-p vpnB` 主动连 B；**不填 `--tunnel-ip`**

```bash
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 vpn \
     -i vpnA -p vpnB -l 19001 -s 119.4.206.14:12345 \
     --local-net 172.16.10.0/23

sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 vpn \
     -i vpnC -p vpnB -l 19002 -s 119.4.206.14:12345 \
     --local-net 172.16.10.0/23
```

**④ 验收（另开终端）**

```bash
# 在 A 上（先看它拿到的地址：应为 10.0.0.1/24，由 B 分配）
ip addr show dev tun0
ping -c 2 10.0.0.254      # 通 B 的隧道地址（交换到的对端隧道地址精确选路）
ping -c 2 10.10.10.115    # 通 B 主机（靠交换到的 10.10.10.0/24 路由）

# 在 B 上
ip addr show dev tun0     # 10.0.0.254/24
ping -c 2 10.0.0.1        # 通 A 的隧道地址（B 自己分配的）
ping -c 2 172.16.10.33    # 通 A 主机（靠交换到的 172.16.10.0/23 路由）
```

> 上面 ping 的是**对端节点自己**的地址（B 本机 `10.10.10.115` / A 本机 `172.16.10.33`），
> 属于"本机收"，两端都不需要 `ip_forward`。若要访问对端网段**里的其它主机**，
> 对端还需 `net.ipv4.ip_forward=1` 并做 SNAT（或给那些主机加回程路由）。

### 2) 同机回环（一个信令服 + 两个节点，验证"分配地址"全流程）

同机没有公网 STUN 可用时，把 `--stun` 指向信令服自身（它兼 STUN 回显）：

```bash
# 终端1：信令服务器（兼 STUN；无需 root）
./sysroot/linux/x86_64/bin/xtools vpnserver -l 127.0.0.1:12345

# 终端2：被动方 B（地址池 = 10.0.0.0/24，自己的地址取 .254）
sudo ./sysroot/linux/x86_64/bin/xtools vpn -i vpnB -s 127.0.0.1:12345 \
     --tunnel-ip 10.0.0.254/24 --local-net 10.10.10.0/24 --stun 127.0.0.1:12345

# 终端3：主动方 A（不填隧道地址；同机回环给个独立设备名更直观）
sudo ./sysroot/linux/x86_64/bin/xtools vpn -i vpnA -p vpnB -s 127.0.0.1:12345 \
     --local-net 172.16.10.0/23 --tun tunA --stun 127.0.0.1:12345
```

打通后：

```bash
ip addr show tunA          # 应出现 10.0.0.1/24（B 分配的）
ip route get 10.0.0.254    # -> dev tunA
ping -c 2 10.0.0.254       # ping B 的隧道地址
```

> 再开一个 `-i vpnC -p vpnB ...` 就能看到 B 依次分配 `.1`、`.2`；
> `Ctrl+C` 掉 C 后，B 会在 ≤5s 打印 `判定断链并回收` + `归还隧道地址 10.0.0.2`；
> 重新起 C 会**复用** `10.0.0.2`。

### 3) mockery 测试命令（单机自检 / 真机 / 回环）

> 用法与 p2p 一致：`xtools <全局选项> mockery <选项> <命令> <参数...>`，`argv[0]` 是命令名。
>
> **三个容易踩的坑**：
>
> 1. **请复制本文的 bash 代码块**，不要从源码注释里复制示例命令——C 注释每行带 ` * `
>    前缀，那个 `*` 会被 shell 通配展开成当前目录一堆文件名，mockery 解析错乱后
>    会把末尾参数当成要 dlopen 的库，然后卡在 `default_producer not ready, waiting...`。
> 2. `test_vpn_peer` 是**常驻阻塞**命令：务必在**独立终端**运行；仅自检时才用
>    `timeout 20 sudo ...` 包裹。
> 3. mockery 会把以 `-` 开头的实参当自己的选项丢弃，所以"随机/自动"请写 `auto`（或 `0`），
>    **不要写 `-`**。

#### 3.1 单机自检（`test_vpn_tun`；需 root）

```bash
# 命令名直接跑，不带任何参数；无需第二个终端/对端（需 root）
sudo ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 test_vpn_tun
```

它是 `REGISTER_TEST_CMD`（**不是** `-f` 函数用例），**不接受命令行参数**；
原因：它需要 root，做成 FUNC 会被"跑全量用例"自动执行、在无特权环境误报失败。
所有取值写死在 [`test_vpn.c`](../../tests/net/test_vpn.c) 顶部的 `#define`：

| 宏 | 默认值 | 含义 | 落在哪条命令 |
| --- | --- | --- | --- |
| `VPN_TUN_TEST_IP` | `10.99.0.1` | 本端 tun 网卡地址（"我是谁"） | `ip addr replace 10.99.0.1/24 dev tun` |
| `VPN_TUN_TEST_MASK` | `24` | 上面地址的掩码 | 拼出 `/24` 前缀 |
| `VPN_TUN_TEST_REMOTE_NET` | `10.99.9.0/24` | "对端网段"（探针目的地） | `ip route replace 10.99.9.0/24 dev tun`（由 `route_add` 装） |
| `VPN_TUN_TEST_NAME` | `NULL` | 设备名，`NULL`=内核分配 `tunN` | `TUNSETIFF` |
| `VPN_TUN_TEST_PROBE_PORT` | `33445` | 探针 UDP 目的端口 | `sendto()` |

内部流程：建 tun → `configure(ip, mask)` 只配地址 → **显式 `route_add(对端网段)`**
（真实场景里这条路由由"交换到的对端网段"产生，单机自测就手工补上）→ 自动向
`10.99.9.2` 发一个 UDP 探针（内核按那条路由交给 tun）→ 从 tun 读回并校验。
出现下面任一行即 PASS：

```
test_vpn_tun: got probe packet N bytes (udp -> 10.99.9.2)
test_vpn_tun: <tunN> PASS (sent=3, got=1)
```

- 探针地址取"对端网段"的**网段基址 + 2**，且**避开本端地址**；
- 两个网段刻意**不同**，探针包只能靠 `route_add` 那条路由进 tun，才验证得到它；
- 返回 `1`=PASS、`0`=未读到(FAIL)、负=环境/权限错误。

#### 3.2 真机跨 NAT（`test_vpn_peer`，与 1) 等价）

位置参数见 3.4；第 5 参是被连方的隧道地址，第 6 参是它自己的内网网段：

```bash
# 服务器（公网 IP，放行 UDP 12345）
./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 test_p2p_server 12345

# 节点 B（被动方）：会话口 12346；隧道地址 10.0.0.254/24（= 地址池）；只填 B 自己的内网
sudo ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 \
     test_vpn_peer vpnB 12346 10.10.10.115 12345 10.0.0.254/24 10.10.10.0/24

# 节点 A（主动方）：会话口 19001；第 5 参写 auto = 隧道地址由 B 分配
sudo ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 \
     test_vpn_peer vpnA 19001 119.4.206.14 12345 auto 172.16.10.0/23 vpnB
```

验收：A 上 `ping 10.0.0.254`（B 的隧道地址）与 `ping 10.10.10.115`（B 主机）通；
B 上 `ping <A 被分配到的地址>`（B 日志里会打印）与 `ping 172.16.10.33` 通。

> 网段写法换算：`255.255.254.0` = `/23`，`255.255.255.0` = `/24`。
> 第 6 参是**本端**内网网段，两端各填自己的；隧道地址由被动方给（`.254`）或分配。

#### 3.3 单机回环（信令服务器兼 STUN）

```bash
# 终端1：信令服务器（同时兼 STUN 采址）
./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 test_p2p_server 9000

# 终端2：B 被动方（隧道地址 10.0.0.254/24 = 地址池；第 6 参 auto=不做内网通告；
#        末尾两参把 STUN 指向信令服自身；auto 占位 <tun_name>）
sudo ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 \
     test_vpn_peer vpnB 12346 127.0.0.1 9000 10.0.0.254/24 auto auto 127.0.0.1 9000

# 终端3：A 主动方（第 5 参 auto = 隧道地址由 B 分配）
sudo ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 \
     test_vpn_peer vpnA 19001 127.0.0.1 9000 auto auto vpnB auto 127.0.0.1 9000
```

打通后（A 的地址由 B 分配，看它日志或 `ip addr show`）：

```bash
ping -c 2 10.0.0.254      # 在 A 上 ping B 的隧道地址
```

#### 3.4 `test_vpn_peer` 参数说明

```
test_vpn_peer <stun_id> <local_service> <signal_host> <signal_port>
              <tunnel_ip> <local_net> [<peer_id> [<tun_name>
              [<stun_host> <stun_port>]]]
```

- `<stun_id>`：本节点标识（需与对方的 `<peer_id>` 互相指认）；
- `<local_service>`：会话(data)口端口；建议固定（如 `12346`/`19001`，便于安全组放行）；`auto`/`0`=随机；
- `<signal_host> <signal_port>`：信令服务器地址（必填）；
- `<tunnel_ip>`：**本端隧道地址**；**被动方必填**（建议 `x.x.x.254/24`，它同时是地址池）；
  `auto`/`0` = 不填，**由对端分配**（仅主动方可用）；
- `<local_net>`：**本端内网网段**（如 `172.16.10.0/23`）；链路建立后自动交换；`auto`/`0`=不通告；
- `<peer_id>`：**带 = 主动方**（主动建链）；**不带 = 被动方**（常驻等被连）；
- `<tun_name>`：设备名，`auto`/省略=内核分配 `tunN`；
- `<stun_host> <stun_port>`：可选，覆盖主 STUN；同机回环可传信令服自身。

STUN 采址缺省用公共 `stun.cloudflare.com:3478`（需网络可达；否则覆盖成信令服）。

## 转发模型

```
建链：p2p 建链 -> 等 CONNECTED -> 交换地址（主叫发 NET_NOTIFY，被动方应答并分配）
      -> 一次性配 tun（本端地址 + 各对端网段路由）-> 进入转发循环
出站：Tun.read(IP 包) -> 按目的地址选链路 -> p2p_session_send
      （主循环 poll tun fd，200ms 超时以响应 Ctrl+C；空闲 tick 里回收断链/补装路由）
入站：p2p recv 回调（带 session，可区分对端）-> Tun.write(注入协议栈)
```

- 出站包直接透传，**不加隧道头**：一个对端一个会话 socket，"这包是谁的"由 socket 决定；
- 选路表来自交换：`link->peer_tun_ip`（对端隧道地址，精确）+ `link->peer_net`（对端内网网段，最长前缀）；
- 未打通 / 选不到链路时**丢弃**出站包（不排队），避免无界缓冲；
- **被动链路**断链会被回收（关会话 + 归还隧道地址 + 空出槽位，可被后来的对端复用）；
  **主动链路**保留（`vpn_run` 只在启动时建一次，回收等于永久断线）；
- `Ctrl+C` → 关全部会话、下线节点、销毁 Tun。Tun 设备是"**进程持有 fd 才存在**"的
  （`open("/dev/net/tun")+TUNSETIFF`，**未设** `TUNSETPERSIST`），所以 `close(fd)` 后内核注销该设备，
  **绑在它上面的地址与路由会一并自动删除**，无需手动 `ip addr del` / `ip route del`。

## 日志与排障

- 成功：`vpn: tun <name> opened (mtu=1400)，待地址交换后配置` →
  `vpn: tun <name> up, tunnel-ip=... mtu=1400` → `vpn: <id> <-> <peer> connected (server-confirmed)`；
- 地址交换（正常各一次）：
  `vpn: 已向对端通告本端内网 <网段>，等对端分配地址`、
  `vpn: 对端已接受本端通告，分配本端隧道地址 <ip/len>`、
  `vpn: 分配隧道地址 <ip> 给新对端` / `vpn: 已接受对端通告，分配 <ip/len> 给对端（本端 <ip>）`、
  `vpn: 对端内网网段 <网段>，加路由`；
- 链路：`vpn: 新增对端链路 slot=N（在线 M 条，主动/被动）`、
  `vpn: 释放链路（归还隧道地址 <ip>）`；
- 路由：`ip route replace <net> dev <tun>` 失败只告警
  （`tun: route ... not added (may be covered by connected route)`）；
- 收不到包先查通道：用 `test_p2p_peer` 或 [`../p2p/README.md`](../p2p/README.md) 的排障（UDP 放行等）；
- MTU：默认 1400，与 p2p 单包上限对齐；两端同值可避免分片丢失。

## 相关

- 设计文档：[`p2p_vpn_design.md`](p2p_vpn_design.md)
- p2p 通道：[`../p2p/README.md`](../p2p/README.md) / [`../p2p/p2p_design.md`](../p2p/p2p_design.md)
