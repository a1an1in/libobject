# VPN 模块使用说明（net/vpn）

基于 [`p2p`](../p2p/README.md) 穿透通道的上层 **站点间网段互访（L3 点对点 VPN）**：
两端各建一个 Linux TUN 虚拟网卡，本机发往对端网段的 IP 包经 p2p 直连通道透传，
对端收到后写入自己的 TUN 注入协议栈，从而 `ping` 通对端 tun IP / 对端内网主机。

设计与取舍见 [`p2p_vpn_design.md`](p2p_vpn_design.md)。

## 文件与职责

| 文件 | 作用 |
| --- | --- |
| [`src/include/libobject/net/vpn/Vpn.h`](../../src/include/libobject/net/vpn/Vpn.h) | **对外接口**：`vpn_cfg_t` + [`vpn_run()`](../../src/include/libobject/net/vpn/Vpn.h:50) |
| [`src/net/vpn/Vpn.c`](../../src/net/vpn/Vpn.c) | 入口实现：Tun ↔ p2p 会话双向转发、建链与退出 |
| [`src/net/vpn/Vpn_Command.c`](../../src/net/vpn/Vpn_Command.c) | **命令行** `xtools vpn ...`（解析选项 → 组装 `vpn_cfg_t` → `vpn_run`） |
| [`src/net/vpn/Vpn_Server_Command.c`](../../src/net/vpn/Vpn_Server_Command.c) | **服务端命令行** `xtools vpnserver ...`（信令+STUN 服务器，复用 `p2p_server_run`） |
| [`src/net/vpn/tun/Tun.h`](../../src/net/vpn/tun/Tun.h) | Tun 抽象（vpn 内部；`open/configure/read/write/set_mtu/close`） |
| [`src/net/vpn/tun/os/unix/Tun.c`](../../src/net/vpn/tun/os/unix/Tun.c) | Linux 实现（`/dev/net/tun`，L3 `IFF_TUN\|IFF_NO_PI`） |
| [`tests/net/test_vpn.c`](../../tests/net/test_vpn.c) | 测试命令：`test_vpn_tun`（Tun 自检）/ `test_vpn_peer`（双端互访） |

分层：`src/net/vpn` 与 `src/net/p2p` **平级并列**，`vpn -> p2p` 单向依赖；VPN 只用 p2p 公共头。
Tun **内聚在 vpn 内部**，等出现第二使用者再上提为公共抽象层（见设计文档 §8）。

## 首版范围

- Linux TUN、**L3 路由模式**、**点对点**；
- 配置地址/路由走**外部 `ip` 命令**；MTU 走 `ioctl`；
- **不加隧道头**（点对点单通道无需区分对端）、**不加加密**；
- 保活由 p2p 会话内部维持（VPN 不感知）。

不做（后续增强）：多 peer 网格与 cidr→peer 路由、TAP/二层 + ARP、Windows TAP、
加密、TURN 中继。

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
# 或增量（新增文件后需先重跑一次 cmake 以刷新 GLOB）
cmake -S . -B build/linux/x86_64 -DPLATFORM=linux && make -C build/linux/x86_64 -j$(nproc)
```

## 命令行（正式入口）

### 服务端：`xtools vpnserver`

VPN 没有独立服务端，它复用 p2p 的中心服务器（同一 UDP 端口兼**信令地址簿/撮合 + STUN 回显**，
预留 TURN）。`vpnserver` 就是它的正式入口（`test_p2p_server` 只是测试命令）：

```bash
# 监听 0.0.0.0:12345（UDP），阻塞运行，Ctrl+C 停止
./sysroot/linux/x86_64/bin/xtools --log-type=0 vpnserver -l 0.0.0.0:12345
ss -lunp | grep 12345        # 确认 UDP 在听
```

| 选项 | 默认值 | 说明 |
| --- | --- | --- |
| `-l, --listen <host:port>` | `0.0.0.0:12345` | 监听地址（UDP）；`:12345` 等价 `0.0.0.0:12345` |

### 节点：`xtools vpn`

```bash
# 节点 A（主叫）：本端 tun 10.0.0.1/24，去 B 侧 10.10.10.0/24，数据口固定 19001
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 vpn \
     -i vpnA -p vpnB -l 19001 -s 119.4.206.14:12345 --ip 10.0.0.1/24 -r 10.10.10.0/24

# 节点 B（被叫）：省略 -p；本端 tun 10.0.0.2/24，去 A 侧 172.16.10.0/23，数据口固定 12346
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 vpn \
     -i vpnB -l 12346 -s 10.10.10.115:12345 --ip 10.0.0.2/24 -r 172.16.10.0/23
```

| 选项 | 必填 | 默认 | 说明 |
| --- | --- | --- | --- |
| `-i, --id <stun_id>` | ✅ | — | 本端 stun id |
| `-p, --peer <stun_id>` | | 空 | 对端 stun id；**省略=被叫**（常驻等被叫） |
| `-s, --signal <host:port>` | ✅ | — | 信令服务器；**必须带端口** |
| `--stun <host[:port]>` | | `stun.cloudflare.com:3478` | 采址用 STUN（探测本机公网地址）；`none`/`off`=禁用（回退到**用信令服采址**） |
| `--stun2 <host[:port]>` | | `stun.l.google.com:19302` | 第二个 STUN（判断 NAT 是否对称）；`none`/`off`=禁用 |
| `-l, --local-service <port>` | | 空=随机 | 本端数据口(UDP)；**防火墙只放行特定端口时必须固定它**（如 `-l 12346`），见「端口与防火墙」 |
| `--ip <a.b.c.d[/len]>` | ✅ | — | **本端隧道地址**（tun 网卡地址）；前缀写在 `/len`（省略按 `/24`）。**两端必须同网段**（A `10.0.0.1/24` + B `10.0.0.2/24`）才开箱即通，否则需两端各自加 `-r` |
| `-r, --route <cidr>` | | 空=不加 | **要访问的对端内网网段**（详见下方说明） |
| `-t, --tun <name>` | | 空=自动 | tun 设备名（自动时内核分配 `tunN`） |
| `--interval <ms>` | | `200` | 打洞/保活周期 |

### `-r/--route` 的本质与怎么填（最常问）

**本质：给本机加一条路由** —— `ip route replace <cidr> dev <tun>`，含义是
"**目的地址落在该网段的包，出接口用 tun**"；这些包随即被丢进 tun，由我们的转发循环读出来、经 p2p 送到对端：

```
你在 A 上填了 -r 192.168.2.0/24，然后访问 192.168.2.10
  └ A 内核选路: 命中 192.168.2.0/24 dev tunA → 包写进 tunA
      └ A vpn: read(tunA) → p2p_session_send ──通道──▶ B
          └ B vpn: write(tunB) → B 内核按自己的路由转给 192.168.2.10
```

所以填的是**目的网段**（"我要去哪"），不是源网段；它只是本机路由表里的一条，
**不创建子网、也不影响对端**（对端要能回来，得它自己有回程路由）。

**填什么 / 在哪台填**——两端各自描述"我要去哪"，方向是交叉的：

| 目标 | 在哪台机器上填 | 填什么 |
| --- | --- | --- |
| 从 A 访问 B 的内网 `192.168.2.0/24` | **A** | `-r 192.168.2.0/24` |
| 从 B 访问 A 的内网 `192.168.1.0/24` | **B** | `-r 192.168.1.0/24` |
| 从 A 访问 B 主机的 LAN IP（如 B 的 `10.10.10.115`） | **A** | `-r 10.10.10.0/24`（那属于"B 的内网网段"） |

**所以实务上按"必填"对待**：VPN 的意义就是访问对端内网，要对端内网（含对端主机的 LAN IP）
就得填 `-r`，否则包会走本机默认网关、根本进不了隧道。只有做**隧道连通性自测**
（只 `ping` 对端隧道地址 `10.0.0.2`）时才不需要——那种情况下隧道段两端本来就直连互通
（`--ip 10.0.0.1/24` 已派生直连路由）；程序在未指定 `-r` 时也会打一条告警提醒。

**通用判据（一句话）**：是否需要 `-r`，只看"本机路由表里有没有到该目的地址的**非默认**路由"。
用 `ip route get <addr>` 一眼可判：

```bash
ip route get 10.0.0.2        # → dev tunA                 ⇒ 已进隧道，不需要 -r（隧道段直连）
ip route get 10.10.10.115    # → via 192.168.1.1 dev eth0 ⇒ 走了本机默认网关，必须加 -r
```

### 为什么"两端隧道地址必须在同一网段"

因为两端隧道地址是**互相直连**的关系，而不是"经过网关转发"：

- `--ip 10.0.0.1/24` 会让内核派生一条直连路由 `10.0.0.0/24 dev tunA`；
- 对端隧道地址 `10.0.0.2` 落在这条路由里 → 内核认为"**链路直连可达**" → 出接口就是 tunA
  → 不需要 `-r`；对端同理（它也有 `10.0.0.0/24 dev tunB`），所以回程也天然通。

推论（也是常见的坑）：

| 两端 `--ip` 配置 | 结果 |
| --- | --- |
| A `10.0.0.1/24` + B `10.0.0.2/24` | ✅ 互相直连通，免 `-r`（**推荐写法**） |
| A `10.0.0.1/24` + B `10.0.0.2/16` | ⚠️ 勉强能通但不对称，`/16` 会吞掉大量地址，别这么配 |
| A `10.0.0.1/32` + B `10.0.0.2/32` | ❌ 没有直连路由，**两端都要**各自加 `-r <对方隧道地址>/32` 才能通 |

> **别把两种"网关"搞混**：
> - **隧道对端地址**（`10.0.0.2`，经典 VPN 里 `... via 10.0.0.2 dev tun` 的那个"网关"）：与本端隧道地址
>   **同段**，默认就走 tun，**不需要 `-r`**；
> - **对端内网的物理网关**（如 B 内网出口 `10.10.10.1`）：与本机 LAN 网关（如 `192.168.1.1`）
>   **不需要、也不应该**同段；本机默认会把它发给自己的默认网关（走 eth0、不进隧道），
>   所以必须靠 `-r 10.10.10.0/24` 把整个对端网段引到 tun 上。

其余要点：

- 想让对端网段**里的其它主机**（不是对端节点自己）也能被访问，对端还需
  `net.ipv4.ip_forward=1`，并在对端做 SNAT（或给那些主机加回程路由）；
- 别把它填成本端自己的网段，也别与隧道段（`10.0.0.0/24`）重叠。

### 端口与防火墙

| 端口 | 是谁 | 放行 |
| --- | --- | --- |
| `<服务器>:12345`（UDP） | 信令 + STUN 服务器（所有节点共用） | 服务器侧放行**入向** UDP 12345 |
| `-l` 指定的**本端数据口**（UDP） | 本端 p2p 会话自己的 socket | 防火墙只放行特定端口时：固定它并放行**入向**（如 `-l 12346`） |
| 出向 UDP | 本机 → 服务器 / 对端 | 一般默认放行；打洞链路靠出向建立、保活维持 |

- `-l` 省略 = 随机端口（NAT 后普通场景够用）；**只有"防火墙只放行特定端口"的机器才必须固定**。
- 启动日志会打印实际生效的端口，便于确认：
  `vpn: id=vpnB peer=(callee) signal=10.10.10.115:12345 service=12346 ...`
- 被叫方也要放行自己的 `-l` 端口（它是被动收包的一方）；服务器端口只有一个，不需要每对端一个。
- 一个 `vpn` 进程当前只连一个对端（点对点单会话）；多对端（每会话需独立端口）属后续"多 peer 网格"，
  本轮先按单端口使用。

### 排障：两端迟迟看不到 `connected`

按可能性排序：

1. **采址 STUN 不合适（最常见）**：若把 `--stun` 指向**与被叫同网段的信令服**，被叫采到的是
   **内网地址**，对端拿这个地址去打洞必然打不通。CLI 默认已用公网 STUN
   （`stun.cloudflare.com:3478`）；也可显式把两端 `--stun` 都指到**公网可达**的 STUN。
   日志锚点：`nat_type=`、`received MATCH from ...`、STUN 目标地址。
2. **UDP 未放行**：服务器侧入向 UDP `12345`；本机出向 UDP（`-l` 只管入向）。
   `sudo tcpdump -i any -n -vv udp port 12345` 确认有来包。
3. **id / peer 不匹配**：A 用 `-p vpnB`；B 必须 `-i vpnB` 且**不带 `-p`**（被叫）。
4. **两端连的不是同一台信令服**：`-s` 的 host 可以是内网口或公网口，但必须是同一台。
5. **双对称 NAT**：两端都 `SYMMETRIC` 时打洞大概率失败（需 TURN，首版预留）。

抓流程用日志锚点：`MATCH` → `KEEPALIVE(打洞)` → `PUNCHOK` → `CONNECTED`，缺哪一环就查哪一环。

`xtools vpn --help` 可看全部选项。**注意**：`vpn`/`vpnserver` 都是常驻命令
（`vpn` 阻塞转发至 Ctrl+C，`vpnserver` 阻塞服务至 Ctrl+C），请在独立终端运行。

## 测试与验证

> **三条路，按需挑一条**：
>
> | 方式 | 命令 | 覆盖范围 | 前置条件 |
> | --- | --- | --- | --- |
> | **命令行（推荐）** | `xtools vpnserver` + `xtools vpn` | 完整隧道（真机跨 NAT，**已实测通过**） | ≥2 台机器，各需 root |
> | mockery 等价写法 | `test_p2p_server` + `test_vpn_peer` | 同上（**同一条实现路径**） | 同上 |
> | 单机自检 | `test_vpn_tun` | 只到 Tun 层（open/configure/read） | 单机 + root |
>
> - `test_vpn_peer` 与 `xtools vpn` **同实现、同默认值**：内部就是构造 `Vpn_Command`、跑同一个
>   `run_command`；区别只是它只能用**位置参数**（mockery 会把 `-` 开头的实参当自己的选项处理，
>   `-i/-s/...` 传不进来）。
> - **完整隧道不能单机测**：同一 netns 里两个 tun 配同网段会被本机直连路由短路；
>   单机能验的只有 Tun 层与 p2p 通道（`-f test_p2p_loopback`）。
> - `vpn` / `vpnserver` / `test_vpn_peer` 都是**常驻阻塞**命令（Ctrl+C 退出），请放在独立终端运行。
> - 下文示例地址：服务器公网 `119.4.206.14` / 内网口 `10.10.10.115`；A 侧内网 `172.16.10.33/23`；
>   B 侧内网 `10.10.10.115/24`。

### 1) 真机跨 NAT：命令行（推荐）

**① 服务端**（放行 UDP 12345）

```bash
./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 vpnserver -l 0.0.0.0:12345
ss -lunp | grep 12345        # 确认 UDP 在听；Ctrl+C 停止
```

**② 节点 B**（被叫；`-s` 用 B 可达的地址；`-l` 固定本端数据口，便于防火墙放行）

```bash
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 vpn \
     -i vpnB -l 12346 -s 10.10.10.115:12345 \
     --ip 10.0.0.2/24 \
     -r 172.16.10.0/23
```

**③ 节点 A**（主叫；`-p vpnB` 表示主动去连 B）

```bash
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 vpn \
     -i vpnA -p vpnB -l 19001 -s 119.4.206.14:12345 \
     --ip 10.0.0.1/24 \
     -r 10.10.10.0/24
```

**④ 验收（另开终端 `ping`）**

```bash
# 在 A 上：
ping -c 2 10.0.0.2        # 通 B 的隧道地址（两端同段，无需 -r）
ping -c 2 10.10.10.115    # 通 B 主机（靠 -r 10.10.10.0/24 把该网段引到 tun）

# 在 B 上：
ping -c 2 10.0.0.1        # 通 A 的隧道地址
ping -c 2 172.16.10.33    # 通 A 主机（靠 -r 172.16.10.0/23）
```

成功标志：两端日志出现
`vpn: <id> <-> <peer> connected (server-confirmed)` 与 `vpn: tunnel up (...)`，
随后 `ping` 有回包。

> 单机自检（无需对端）见下文 **2.1 单机自检**。

> 以上 ④ 里 ping 的是**对端节点自己**的地址（B 本机 `10.10.10.115` / A 本机 `172.16.10.33`），
> 属于"本机收"，两端都不需要 `ip_forward`。若要访问对端网段**里的其它主机**，
> 对端还需 `net.ipv4.ip_forward=1` 并做 SNAT（或给那些主机加回程路由）。

### 2) mockery 测试命令（单机自检 / 真机 / 回环）

> 用法与 p2p 一致：`xtools <全局选项> mockery <选项> <命令> <参数...>`，`argv[0]` 是命令名；
> 统一加 `--log-type=0` 让日志输出到控制台。
>
> **三个容易踩的坑**：
>
> 1. **请复制本文的 bash 代码块**，不要从源码注释里复制示例命令——C 注释每行带 ` * `
>    前缀，那个 `*` 会被 shell 通配展开成当前目录一堆文件名，mockery 解析错乱后
>    会把末尾参数(如 `172.16.10.0/23`)当成要 dlopen 的库，然后卡在
>    `default_producer not ready, waiting...`。
> 2. `test_vpn_peer` 是**常驻阻塞**命令（设计如此，Ctrl+C 退出）：被叫会一直等被叫。
>    务必在**独立终端**运行，不要前台裸跑；仅自检时才用 `timeout 20 sudo ...` 包裹。
> 3. mockery 的参数解析会把以 `-` 开头的实参当作选项丢弃，因此"随机/自动"请写
>    `auto`（或 `0`），**不要写 `-`**（旧 p2p 文档里的 `-` 实际会被吞掉）。

#### 2.1 单机自检（`test_vpn_tun`；需 root）

```bash
# 命令名直接跑，不带任何参数；无需第二个终端/对端（需 root）
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_vpn_tun
```

它是 `REGISTER_TEST_CMD`（**不是** `-f` 函数用例），**不接受命令行参数**（无参可配）；
原因：它需要 root，做成 FUNC 会被"跑全量用例"自动执行、在无特权环境误报失败。
所有取值写死在 [`test_vpn.c`](../../tests/net/test_vpn.c) 顶部的 `#define`，要改就改那里再重新编译：

| 宏 | 默认值 | 含义 | 落在哪条命令 |
| --- | --- | --- | --- |
| `VPN_TUN_TEST_IP` | `10.99.0.1` | 本端 tun 网卡地址（"我是谁"） | `ip addr replace 10.99.0.1/24 dev tun` |
| `VPN_TUN_TEST_MASK` | `24` | 上面地址的掩码（也可写 `255.255.255.0`） | 拼出 `/24` 前缀 |
| `VPN_TUN_TEST_REMOTE_CIDR` | `10.99.9.0/24` | 路由目的网段（"我要去哪"） | `ip route replace 10.99.9.0/24 dev tun` |
| `VPN_TUN_TEST_NAME` | `NULL` | 设备名，`NULL`=内核自动分配 `tunN` | `TUNSETIFF` |
| `VPN_TUN_TEST_PROBE_PORT` | `33445` | 探针 UDP 目的端口 | `sendto()` |

内部流程：建 tun → 配 `10.99.0.1/24` → 按 `remote_cidr` 加路由 → 自动向
`10.99.9.2` 发一个 UDP 探针包（内核按那条路由交给 tun）→ 从 tun 读回并校验
（IPv4 + UDP + 目的地址）。出现下面任一行即 PASS：

```
test_vpn_tun: got probe packet N bytes (udp -> 10.99.9.2)
test_vpn_tun: <tunN> PASS (sent=3, got=1)
```

- 探针地址取 `remote_cidr` 的**网段基址 + 2**（避开 `.0`/`.1` 常用网关），且**避开本端地址**；
- 两个网段刻意**不同**，这样探针包只能靠 `ip route replace <cidr> dev <tun>` 进 tun，
  才是对 `remote_cidr` 的真实验证（若写成同网段，走的是 `addr` 派生的直连路由，测不到它）；
- 返回 `1`=PASS、`0`=未读到(FAIL)、负=环境/权限错误；无 root 时报
  `test_vpn_tun: open failed (need root/CAP_NET_ADMIN?)`。

> 手动辅助观察也行：配好 tun 后在另开终端 `ping 10.99.9.2`，同样能看到 Tun 读到包——
> 但自动化判定以 `test_vpn_tun` 自身为准。

#### 2.2 真机跨 NAT（`test_vpn_peer`，与 1) 等价）

> `test_vpn_peer` **复用正式命令行实现**：内部构造 `Vpn_Command`、把位置参数逐项写进它的选项，
> 再跑同一条 `run_command`——所以默认值（公网 STUN 等）、校验、转发逻辑都只有一份，
> 不会像早先那样出现"测试与 CLI 行为不一致"。等价命令行见「实际测试命令」一节。
>
> 之所以用**位置参数**：mockery 会把以 `-` 开头的实参当自己的选项处理，`-i/-s/...` 传不进来。

```bash
# 服务器（公网 IP，放行 UDP 12345）
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 12345

# 节点 B（被叫）：会话口固定 12346；本端 tun 10.0.0.2，要访问 A 侧 172.16.10.0/23
#   <signal_host> 填 B 这边可达的服务器地址（B 在 10.10.10.0/24，服务器内网口 10.10.10.115）
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
     test_vpn_peer vpnB 12346 10.10.10.115 12345 10.0.0.2 172.16.10.0/23

# 节点 A（主叫）：会话口固定 19001；本端 tun 10.0.0.1，要访问 B 侧 10.10.10.0/24
#   （A 侧内网 172.16.10.33/23 —— netmask 255.255.254.0 即 /23）
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
     test_vpn_peer vpnA 19001 119.4.206.14 12345 10.0.0.1 10.10.10.0/24 vpnB
```

验收：A 上 `ping 10.0.0.2`（B 的 tun）与 `ping 10.10.10.115`（B 主机）通；
B 上 `ping 10.0.0.1`（A 的 tun）与 `ping 172.16.10.33`（A 主机）通。
STUN 采址用默认公共服务器（同 `test_p2p_peer` 真机示例）。

> 掩码换算：`255.255.254.0` = `172.16.10.0/23`（覆盖 172.16.10.0 ~ 172.16.11.255），
> `255.255.255.0` = `/24`。`<remote_cidr>` 只填**对端**网段，本端网段由本机直连路由负责。
> 隧道段独立于两侧内网（示例用 10.0.0.0/24，两端各 10.0.0.1 / 10.0.0.2）。

#### 2.3 单机回环（信令服务器兼 STUN；对应 CLI 加 `--stun 127.0.0.1:9000`）

```bash
# 终端1：信令服务器（同时兼 STUN 采址，同机回环无需公网 STUN）
./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 test_p2p_server 9000

# 终端2：B 被叫（会话口 12346；本端 tun 10.0.0.2）
#   末尾 <stun_host> <stun_port> 覆盖为信令服自身；中间的 auto 占位 <tun_name>
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
     test_vpn_peer vpnB 12346 127.0.0.1 9000 10.0.0.2 10.0.0.0/24 auto 127.0.0.1 9000

# 终端3：A 主叫（会话口 19001；本端 tun 10.0.0.1）
sudo ./sysroot/linux/x86_64/bin/xtools --log-type=0 mockery --log-level=0x16 \
     test_vpn_peer vpnA 19001 127.0.0.1 9000 10.0.0.1 10.0.0.0/24 vpnB auto 127.0.0.1 9000
```

> 无公网时把主 STUN 覆盖为信令服务器自身（它兼 STUN 回显）：即末尾
> `<stun_host> <stun_port>` 传 `127.0.0.1 9000`；`<tun_name>` 位置写 `auto` 占位。
> 有公网时省略这两个参数、用默认公共 STUN（见上一节真机示例）。

打通后：

```bash
ping 10.0.0.2        # 在 A 上 ping B 的 tun IP
```

#### 2.4 `test_vpn_peer` 参数说明

```
test_vpn_peer <stun_id> <local_service> <signal_host> <signal_port>
              <local_ip> <remote_cidr> [<peer_id> [<tun_name>
              [<stun_host> <stun_port>]]]
```

- `<stun_id>`：本节点标识（需与对方的 `<peer_id>` 互相指认）；
- `<local_service>`：会话(data)口端口；**建议固定端口**（如 `12346`/`19001`，与
  `test_p2p_peer` 真机示例一致，便于安全组放行与排障）；`auto`/`0`=随机；
- `<signal_host> <signal_port>`：信令服务器地址（必填）；
- `<local_ip>`：**本端 tun IP**（会 `ip addr replace <local_ip>/<prefix> dev <tun>`）；
- `<remote_cidr>`：**对端网段**，按需 `ip route replace <cidr> dev <tun>`；
  两端若同网段（都配 `x.x.x.0/24`）直连路由已覆盖，无需额外路由；
- `<peer_id>`：**带 = 主叫**（主动建链）；**不带 = 被叫**（常驻等被叫）；
- `<tun_name>`：设备名，`auto`/省略=内核分配 `tunN`；
- `<stun_host> <stun_port>`：可选，覆盖主 STUN（采址用）；同机回环可传信令服自身。

STUN 采址缺省用公共 `stun.cloudflare.com:3478`（需网络可达；否则覆盖成信令服）。

## 转发模型

```
出站：Tun.read(IP 包) -> p2p_session_send          （主循环 poll tun fd，200ms 超时以响应 Ctrl+C）
入站：p2p recv 回调 -> Tun.write(注入协议栈)         （p2p 事件线程；回调内不阻塞）
```

- 出站包直接透传，**不加隧道头**（点对点单通道）；
- **被叫**没有 `p2p_session_create`，其会话句柄是在**收到对端第一个包时**从 recv 回调
  里"学到"的，之后即可出站回发（日志 `vpn: learned peer session (callee path)`）；
- 未打通时丢弃出站包（不排队），避免无界缓冲；
- 等打通：主叫等服务器 `CONNECTED`（60s 超时失败）；被叫常驻不超时；
- `Ctrl+C` → 关会话、下线节点、销毁 Tun。Tun 设备是"**进程持有 fd 才存在**"的
  （我们用 `open("/dev/net/tun")+TUNSETIFF`，**未设** `TUNSETPERSIST`），所以
  `close(fd)` 后内核注销该设备，**绑在它上面的地址与路由会一并自动删除**，
  无需手动 `ip addr del` / `ip route del`。若你另用 `ip tuntap add ... mode tun`
  建过同名**持久**设备，才会残留。

## 日志与排障

- 成功：`vpn: tun <name> up, ip=... remote=... mtu=1400`、
  `vpn: <id> <-> <peer> connected (server-confirmed)`、`vpn: tunnel up`；
- 权限：见上文"权限要求"；
- 路由：`ip route replace <cidr> dev <tun>` 失败只告警
  （`tun: route ... not added (may be covered by connected route)`）；
- 收不到包先查通道：用 `test_p2p_peer` 或 `doc/net/p2p/README.md` 的排障（UDP 放行等）；
- MTU：默认 1400，与 p2p 单包上限对齐；两端同值可避免分片丢失。

## 相关

- 设计文档：[`p2p_vpn_design.md`](p2p_vpn_design.md)
- p2p 通道：[`../p2p/README.md`](../p2p/README.md) / [`../p2p/p2p_design.md`](../p2p/p2p_design.md)
