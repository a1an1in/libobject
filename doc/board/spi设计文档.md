# SPI 驱动设计文档

## 1. 概述

基于 Linux spidev 子系统实现用户空间 SPI 驱动（Spi 类），提供通用的 SPI 总线访问接口。
在 QEMU virt 中添加 PL022 SPI 控制器 + n25q064 SPI NOR Flash + spidev 用于测试。

### 1.1 Spi 类 vs Mtd 类：什么时候用哪个

SPI 设备在内核中有两种访问方式，取决于**有没有内核驱动**：

```
场景 1: 有内核驱动 → 不需要 Spi 类
────────────────────────────────────
  应用      Mtd 类 → /dev/mtd1 → MTD 子系统 → m25p80 驱动 → SPI → Flash
  内核      m25p80 驱动自动处理 SPI 通信
  适用      n25q064、w25q64 等标准 SPI Flash

场景 2: 无内核驱动 → 必须用 Spi 类
────────────────────────────────────
  应用      Spi 类 → /dev/spidevX.Y → spidev 驱动 → SPI → 自定义设备
  内核      spidev 只做数据转发，不理解数据含义
  适用      FPGA 加载、自研传感器、私有协议 SPI 外设
```

| | Mtd 类 | Spi 类 |
|---|---|---|
| **操作对象** | `/dev/mtdN`（MTD 字符设备） | `/dev/spidevX.Y`（SPI 用户空间接口） |
| **使用条件** | 设备有内核驱动（如 SPI Flash） | 设备无内核驱动（自定义外设） |
| **接口** | erase/write/read/write_file | transfer/write/read/write_then_read |
| **典型场景** | 刷固件、读写 Flash 数据 | FPGA bitstream 加载、用户态设备驱动 |

> **关键理解**：SPI 只是传输方式。如果内核认识这个 SPI 设备（如 Flash），就用内核驱动的接口（MTD）。如果内核不认识（自研设备），就用 spidev 从用户空间直接控制 SPI 总线。

### 1.2 测试环境：PL022 + n25q064 + spidev

```
硬件:     PL022 → CS0 → n25q064 NOR Flash (8MB)
设备树:   compatible = "rohm,dh2228fv" → spidev 驱动绑定
结果:     /dev/spidevX.0 → 用户空间直接控制 n25q064
```

| 组件 | 作用 |
|------|------|
| **PL022** | SPI 控制器（Master），产生时钟、管理片选 |
| **n25q064** | 真实 SPI NOR Flash 硬件（8MB），响应 0x9F/0x03/0x02 等标准命令 |
| **spidev** | 内核驱动绑定到 CS0（compatible = "rohm,dh2228fv"），暴露 `/dev/spidevX.0` |

> **设计意图**：n25q064 是真实硬件（响应 SPI 命令），但内核 m25p80 驱动不绑定（不在设备树声明 `jedec,spi-nor`）。spidev 驱动绑定后，用户态通过 Spi 类直接控制 Flash——这就是用户态设备驱动的完整实践。

### 1.3 n25q064 SPI Flash

8MB SPI NOR Flash，支持的 SPI 命令：

| 命令 | 说明 |
|------|------|
| `0x9F` | 读 JEDEC ID（返回 0x20 0xBA 0x17） |
| `0x03` | 读数据（发 3 字节地址 → 读回数据） |
| `0x06` | 写使能 |
| `0x02` | 页编程（最多 256 字节） |
| `0x05` | 读状态寄存器 |

## 2. Spi 类接口

### 2.1 接口列表

| 接口 | 说明 |
|------|------|
| `open(spi, bus, cs)` | 打开 `/dev/spidevB.C`，配置默认参数 |
| `close(spi)` | 关闭设备 |
| `transfer(spi, trs, nmsg)` | 核心：多消息原子传输（SPI_IOC_MESSAGE） |
| `write(spi, tx, len)` | 仅写 |
| `read(spi, rx, len)` | 仅读（发送 dummy 0x00） |
| `write_then_read(spi, tx, tx_len, rx, rx_len)` | 原子写+读，CS 全程保持 |
| `configure(spi, config)` | 配置 mode/speed/bits/lsb |

### 2.2 使用示例

```c
Spi *spi = object_new(allocator, "Spi", NULL);
spi->open(spi, 0, 0);  // bus=0, cs=0 (spidev, 总线号由 /aliases 的 spi0 固定)

// 读 JEDEC ID
uint8_t cmd = 0x9F;
uint8_t id[3];
spi->write_then_read(spi, &cmd, 1, id, 3);  // 发 0x9F，读回 3 字节制造商/设备 ID

spi->close(spi);
```

### 2.3 设计特点

- 与 I2c/Mtd 类风格一致，直接使用 Linux 原生类型（`struct spi_ioc_transfer`）
- `transfer()` 支持多条消息原子组合（如"写命令+读数据"，CS 保持）
- `write_then_read()` 封装最常见的 SPI 操作模式
- 线程安全（pthread_mutex + fd 复用）

### 2.4 典型 SPI 设备使用示例

SPI 是**全双工、主从式**总线（SCLK/MOSI/MISO/CS 四线），用户态通过
`/dev/spidevX.Y` 操作。绝大多数 SPI 设备都遵循"**发命令 →（可选发地址）→
读/写数据**"的模式：

| 操作模式 | 典型场景 | 对应接口 |
|----------|----------|----------|
| 读 ID / 状态 | 发 1 字节命令，读回 N 字节 | `write_then_read(cmd, 1, buf, N)` |
| 读寄存器/读数据 | 发命令+地址，读回数据（CS 全程保持） | `write_then_read(cmd+addr, 4, buf, N)` |
| 写命令 | 仅发命令（如写使能 0x06） | `write(cmd, 1)` |
| 写寄存器/写数据 | 发命令+地址+数据 | `write(cmd+addr+data, len)` |
| 复杂时序 | 一次事务多段读写（dummy、多页等） | `transfer(trs, nmsg)` |

**示例：SPI NOR Flash 完整读写**（原始 spidev 用法，参考代码）：

```c
/* 关键：这块 flash 绑定的是 spidev（不是 m25p80），只是原始字节流，
 * 没有 /dev/mtdN、没有块设备、没有文件系统。要当存储用必须自己
 * 实现 Flash 语义：先擦除(1→全1) → 再编程(1→0) → 读回。 */
#define CMD_READ_ID   0x9F   /* 读 JEDEC ID */
#define CMD_WRITE_EN  0x06   /* 写使能（擦除/编程前必须） */
#define CMD_ERASE_4K  0x20   /* 4KB 扇区擦除 */
#define CMD_PAGE_PROG 0x02   /* 页编程 */
#define CMD_READ_DATA 0x03   /* 读数据 */
#define CMD_READ_STAT 0x05   /* 读状态寄存器 */
#define ST_WIP        0x01   /* 忙位 */

Spi *spi = object_new(allocator, "Spi", NULL);
spi->open(spi, 0, 0);                        /* /dev/spidev0.0 */

uint8_t cmd, id[3], st, wbuf[8], rbuf[4];

cmd = CMD_READ_ID;
spi->write_then_read(spi, &cmd, 1, id, 3);   /* 1. 读 JEDEC ID */

cmd = CMD_WRITE_EN;
spi->write(spi, &cmd, 1);                    /* 2. 写使能 */
wbuf[0] = CMD_ERASE_4K; wbuf[1]=0; wbuf[2]=0; wbuf[3]=0;
spi->write(spi, wbuf, 4);                    /* 3. 擦除 4KB 扇区 */
cmd = CMD_READ_STAT;
do { spi->write_then_read(spi, &cmd, 1, &st, 1); } while (st & ST_WIP); /* 4. 等擦除完 */

cmd = CMD_WRITE_EN;
spi->write(spi, &cmd, 1);                    /* 5. 写使能 */
wbuf[0] = CMD_PAGE_PROG; wbuf[1]=0; wbuf[2]=0; wbuf[3]=0;
wbuf[4]=0xDE; wbuf[5]=0xAD; wbuf[6]=0xBE; wbuf[7]=0xEF;
spi->write(spi, wbuf, 8);                    /* 6. 页编程 */
cmd = CMD_READ_STAT;
do { spi->write_then_read(spi, &cmd, 1, &st, 1); } while (st & ST_WIP); /* 7. 等写完 */

uint8_t raddr[4] = {CMD_READ_DATA, 0, 0, 0};
spi->write_then_read(spi, raddr, 4, rbuf, 4); /* 8. 读回 → rbuf == wbuf[4..7] */

spi->close(spi);
```

**"这个设备怎么用？——它不是块设备"**

上面这种绑定 spidev 的 flash 只是**原始字节流**，要真正"用起来"有两条路线：

| 路线 | 怎么用 | 适用场景 |
|------|--------|----------|
| **A. 作为 MTD 块设备** | 设备树把 flash 声明为 `jedec,spi-nor`，绑定内核 `m25p80` 驱动 → 出现 `/dev/mtdN`（及 `/dev/mtdblockN`）→ 用 [`Mtd`](../src/include/libobject/board/hal/mtd/Mtd.h) 类（erase/write/read/write_file）或挂载 jffs2/ubifs 文件系统 | 需要存储/文件系统，且接受内核驱动管理 |
| **B. 作为原始 spidev 设备**（当前 demo） | 用户态直接用 `Spi` 类发命令：**先擦除 → 再编程 → 读**。要在其上做文件系统，需自己实现一层简单 FTL/文件系统 | 内核不认识的私有设备、FPGA、自研传感器；或想完全掌控 Flash 语义 |

> **为什么当前 demo 不把 flash 挂成块设备**：设计目标正是演示"内核不认识设备时，
> 用户态通过 spidev 直接驱动"这条路线（见 1.1 节）。若需要块设备/文件系统，
> 把设备树 `compatible` 从 `"rohm,dh2228fv"`（spidev）改为 `"jedec,spi-nor"`（m25p80），
> 再用 `Mtd` 类访问同一块物理 flash 即可，两条路线共用同一硬件。

## 3. QEMU virt SPI 实现

### 3.1 硬件布局

```
PL022 SPI 控制器 (0x090d0000, IRQ 72)
└── CS0: n25q064 + spidev (rohm,dh2228fv)
```

### 3.2 QEMU 修改清单

| 文件 | 修改 |
|------|------|
| `include/hw/arm/virt.h` | 添加 `VIRT_SPI` 枚举 |
| `hw/arm/virt.c` | memmap(0x090d0000) + irqmap(72) + `create_spi()` |
| `hw/arm/virt.c` | `create_spi()` 内通过 `/aliases` 的 `spi0` 固定总线号为 0 |
| `hw/arm/Kconfig` | `select PL022` |
| `hw/arm/virt.c` | CS0 挂载 `n25q064`（`qdev_new("n25q064")`）作为真实 SPI NOR Flash |

### 3.3 设备树（QEMU 自动生成）

```dts
/aliases {
    spi0 = &spi@90d0000;   /* 固定 SPI 总线号为 0 → /dev/spidev0.0 */
};

spi@90d0000 {
    compatible = "arm,pl022", "arm,primecell";
    reg = <0x0 0x90d0000 0x0 0x1000>;
    interrupts = <0 72 4>;
    clocks = <&apb_pclk>;
    clock-names = "apb_pclk";
    num-cs = <1>;
    #address-cells = <1>;
    #size-cells = <0>;

    spidev@0 {
        compatible = "rohm,dh2228fv";
        reg = <0>;
        spi-max-frequency = <12000000>;
    };
};
```

> **注意**：PL022 是 AMBA PrimeCell 设备，compatible 必须包含 `"arm,primecell"`。
> spidev 在 Linux 4.9 中只匹配 `"rohm,dh2228fv"`。

## 4. 内核配置

```
CONFIG_SPI=y
CONFIG_SPI_PL022=y
CONFIG_SPI_SPIDEV=y
```

## 5. Guest 设备映射

```
/dev/spidev0.0         # spidev on CS0 (总线号通过 /aliases 的 spi0 固定为 0)
```

### 5.1 SPI 总线号分配原理

`/dev/spidevB.C` 中的 `B` 是 SPI 控制器的**总线号**，由 Linux SPI 核心在控制器注册时确定。
它**不是硬件地址，而是内核分配的逻辑编号**，来源有三种：

| 来源 | 说明 | 内核示例 |
|------|------|----------|
| 平台数据 / 设备 id | 驱动显式指定固定编号 | `spi-rockchip.c`: `master->bus_num = pdev->id` |
| 设备树别名 | 通过 `of_alias_get_id(np, "spi")` 取 `/aliases` 中的编号 | `spi-s3c64xx.c` 显式调用 |
| 动态分配 | `bus_num < 0` 时由 SPI 核心按 `SPI_DYNAMIC_BUS_START` 递减 | PL022（DT 模式）等 |

核心逻辑位于 `spi_register_master()`（`drivers/spi/spi.c`）：

```c
static atomic_t dyn_bus_id = ATOMIC_INIT((1<<15) - 1);   /* = 32767 */

if ((master->bus_num < 0) && master->dev.of_node)
    master->bus_num = of_alias_get_id(master->dev.of_node, "spi"); /* 查 /aliases 的 spiN */

if (master->bus_num < 0) {                 /* 仍 < 0 → 动态分配 */
    master->bus_num = atomic_dec_return(&dyn_bus_id);  /* 先减再取，第一个 = 32766 */
    dynamic = 1;
}
dev_set_name(&master->dev, "spi%u", master->bus_num);
```

- `(1<<15) - 1 = 32767`，`atomic_dec_return` 先减后取，因此第一个动态控制器得到
  **32766**（即 `SPI_DYNAMIC_BUS_START`），后续每个控制器再递减。
  这正是 `/dev/spidev32766.0` 的由来。
- 总线号的唯一性由 `device_add()` 以 `spi%u` 命名保证，与硬件地址无关。

**PL022 的 `bus_num` 来源**（`drivers/spi/spi-pl022.c`）：

```c
master->bus_num = platform_info->bus_id;   /* probe 中直接赋值 */
```

- 有 platform data 时：`bus_id` 来自板级数据，编号固定。
- DT 探测时：`pl022_platform_data_dt_get()` 把 `bus_id` 置为 `-1`，
  于是落入核心的 alias 查询；查不到 alias 就退回 32766 动态分配。

**如何固定总线号**：在设备树 `/aliases` 节点声明 `spi0 = &spi@90d0000;`，
`of_alias_get_id(np, "spi")` 解析到 0，得到确定的 `/dev/spidev0.0`。
QEMU 侧由 `create_spi()` 通过 `qemu_fdt_setprop_string()` 写入该别名。

## 6. 完整 QEMU 启动命令

```bash
qemu-system-aarch64 \
  -M virt -cpu cortex-a57 -m 2G \
  -kernel ~/workspace/linux-4.9.263/arch/arm64/boot/Image \
  -initrd ~/workspace/busybox-1.33.1/initramfs.cpio.gz \
  -nographic \
  -drive file=/home/alan/workspace/qemu_virt_machine/flash.img,if=pflash,index=1,format=raw \
  -virtfs local,path=/home/alan/workspace/libobject/sysroot/linux/aarch64,mount_tag=host0,security_model=none,id=host0 \
  -append "console=ttyAMA0 rdinit=/linuxrc"
```

## 7. 测试

### 7.1 手动验证

```sh
ls /dev/spidev*              # /dev/spidev0.0
dmesg | grep pl022            # ssp-pl022 90d0000.spi: ARM PL022 driver
```

### 7.2 自动化测试

```sh
mkdir -p /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt
ls /mnt/bin/xtools
export LD_LIBRARY_PATH=/mnt/lib
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_spi
```

测试场景：通过 spidev 读 JEDEC ID 的原子操作（`write_then_read`）。

## 8. 文件清单

| 文件 | 说明 |
|------|------|
| `src/include/libobject/board/hal/spi/Spi.h` | SPI 驱动头文件 |
| `src/board/hal/spi/Spi.c` | Linux spidev 后端实现 |
| `tests/board/test_spi.c` | 测试用例 |
| `doc/board/spi设计文档.md` | 本文档 |
