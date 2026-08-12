# UIO FPGA 应用层驱动开发

> 本文从 [`Ubuntu 环境里搭建qemu开发环境.md`](<Ubuntu 环境里搭建qemu开发环境.md>) 的「4. 开发UIO 应用层驱动」一节
> 独立成文，专门介绍如何开发 **UIO FPGA 驱动**：从 QEMU 里造一个会触发中断的虚拟 FPGA，
> 到内核启用 UIO，再到 libobject 里用 `Uio_Fpga` / `Uio_Pcie_Fpga` 类做用户态驱动并测试。

## 1. 概述：为什么 FPGA 用 UIO

**UIO（Userspace I/O）** 是一种"内核只留一个小驱动、把寄存器/中断直接暴露给用户态"的模型：
内核侧的 `uio_pdrv_genirq` 只需按设备树把 MMIO 和中断导出为 `/dev/uioX`，真正的寄存器读写、
业务逻辑全部在用户态完成。

FPGA 特别适合 UIO，因为 FPGA 的寄存器接口随逻辑频繁变更——每次改寄存器都重写、重编内核驱动
代价太高；UIO 让用户态驱动随 FPGA 逻辑一起迭代，内核配置一次（`CONFIG_UIO_PDRV_GENIRQ`）即可。

```
用户态 libobject 驱动 (Uio_Fpga / Uio_Pcie_Fpga)
        │  read/write_register / register_irq
        ▼
/dev/uioX  ←── uio_pdrv_genirq（内核，按设备树导出 map + 中断）
        │
        ▼
设备树节点（compatible = "generic-uio"）→ 硬件 / QEMU 模拟设备（vfpga）
```

libobject 里有两层 FPGA 类：

| 类 | 继承 | 适用 | 头文件 |
|----|------|------|--------|
| `Uio_Fpga` | `Uio` | 平台设备（DT 节点，如本 QEMU 的 `fpga@b000000`，SPI 中断） | [`Uio_Fpga.h`](../src/include/libobject/board/hal/uio/Uio_Fpga.h:26) |
| `Uio_Pcie_Fpga` | `Uio_Pcie` | PCIe 板卡上的 FPGA（BAR = region，INTx/MSI 中断） | [`Uio_Pcie_Fpga.h`](../src/include/libobject/board/hal/uio/Uio_Pcie_Fpga.h:27) |

> 本环境用 QEMU `virt` 机器的平台 UIO 路径（`Uio_Fpga`）演示；PCIe FPGA 的 UIO 路径（`Uio_Pcie_Fpga`）
> 接口类似，只是设备发现走 PCIe BDF、中断走 VFIO 式 eventfd/io_worker。

## 2. 总体流程（四步）

1. **创建虚拟 fpga 设备**：改 QEMU 源码加 `vfpga` 模拟设备 + 改设备树，让 guest 有一个会触发中断的 FPGA。
2. **修改 Linux 内核**：打开 `CONFIG_UIO=y`、`CONFIG_UIO_PDRV_GENIRQ=y` 并重编内核。
3. **实现 libobject FPGA 驱动**：用 `Uio_Fpga` 类（open_device / 寄存器读写 / 中断）。
4. **测试**：手动验证（devmem 触发中断）+ case 测试（`test_uio_fpga_*`）。

## 3. 创建虚拟 fpga 设备（QEMU）

创建虚拟 fpga 设备需要两步：**修改 QEMU 源码添加 `vfpga` 模拟设备**（让 QEMU 能模拟一个会触发中断的
FPGA），**修改设备树**（让 guest 内核识别该设备）。

### 3.1 修改 QEMU 添加 vfpga 模拟设备

QEMU `virt` 机器默认没有 FPGA 外设模型，`0x0b000000` 地址上没有任何设备，写寄存器不会触发中断。
因此需要在 QEMU 源码中新增一个 `vfpga`（Virtual FPGA）模拟设备，guest 写其寄存器即可触发 SPI 70 中断。

修改涉及以下文件：

| 文件 | 修改内容 |
|------|---------|
| `include/hw/misc/vfpga.h` | 新增：vfpga 设备模型头文件（定义寄存器偏移、状态结构） |
| `hw/misc/vfpga.c` | 新增：vfpga 设备模型实现（MMIO 读写、中断控制） |
| `include/hw/arm/virt.h` | 添加 `VIRT_FPGA` 枚举项 |
| `hw/arm/virt.c` | 分配 MMIO 地址 `0x0b000000`、连接 GIC SPI 70、生成设备树节点、调用 `create_vfpga()` |
| `hw/misc/Kconfig` | 添加 `VFPGA` 配置 |
| `hw/misc/meson.build` | 添加编译条目 |
| `hw/arm/Kconfig` | `ARM_VIRT` 添加 `select VFPGA` |

`vfpga` 设备模型的核心逻辑（`hw/misc/vfpga.c`）：

```c
/* 中断控制寄存器 0xFF0：写 bit0=1 触发中断，bit0=0 清除 */
case VFPGA_REG_IRQ_CTRL:   // 0xFF0
    if (value & 1) {
        s->irq_pending = 1;
        qemu_set_irq(s->irq, 1);   // 拉高中断线 → GIC 收到 SPI 70
    } else {
        s->irq_pending = 0;
        qemu_set_irq(s->irq, 0);   // 拉低中断线
    }
    break;
```

在 `hw/arm/virt.c` 中，将设备的中断输出连接到 GIC 的 SPI 70：

```c
static void create_vfpga(const VirtMachineState *vms)
{
    ...
    s = SYS_BUS_DEVICE(qdev_new(TYPE_VFPGA));
    sysbus_realize_and_unref(s, &error_fatal);
    sysbus_mmio_map(s, 0, base);                       // 映射到 0x0b000000
    sysbus_connect_irq(s, 0, qdev_get_gpio_in(vms->gic, irq));  // 连接 GIC SPI 70
    ...
}
```

修改完成后重新编译 QEMU：

```bash
cd ~/workspace/qemu/build
ninja qemu-system-aarch64
```

> **说明**：`vfpga` 设备寄存器布局（相对 `0x0b000000`）：`0x00`-`0xFEC` 为数据寄存器（支持 32/64 位
> 读写），`0xFF0` 为中断控制寄存器（写 bit0=1 触发中断），`0xFF4` 为中断状态寄存器（只读）。

### 3.2 导出默认设备树

```sh
qemu-system-aarch64 \
  -M virt,dumpdtb=virt_default.dtb \
  -cpu cortex-a57 \
  -m 2G \
  -kernel ~/workspace/linux-4.9.263/arch/arm64/boot/Image \
  -nographic
```

### 3.3 反编译为可编辑格式

```sh
dtc -I dtb -O dts virt_default.dtb -o virt_custom.dts
```

### 3.4 修改设备树节点

```
对于你的虚拟 FPGA 设备，建议使用 70，这个编号明确未被占用且远离已分配区域：
fpga@b000000 {
    compatible = "generic-uio";
    reg = <0x0 0xb000000 0x0 0x1000>;
    interrupts = <0 70 4>;
    interrupt-parent = <&intc>;
};
```

> **注意**：`0x0b000000` 是 QEMU `virt` 机器中为 FPGA 模拟设备预留的 MMIO 地址（见 `hw/arm/virt.c` 的
> `VIRT_FPGA`）。该地址对应 QEMU 中新增的 `vfpga` 模拟设备（`hw/misc/vfpga.c`），guest 写其 `0xFF0`
> 寄存器（bit0=1）即可触发 SPI 70 中断。若使用 QEMU 自动生成的设备树，该节点会自动生成，无需手动添加。

### 3.5 编译回 DTB

```sh
dtc -I dts -O dtb virt_custom.dts -o virt_custom.dtb
```

## 4. 修改 Linux 内核（启用 UIO 驱动）

许多嵌入式或通用内核的默认配置为了精简体积，会关闭非核心驱动。请按照以下步骤强制启用 UIO 支持。

### 4.1 直接修改 `.config` 文件

打开内核根目录下的 `.config` 文件，找到以下行（如果存在）：

```
# CONFIG_UIO is not set
```

将其修改为：

```
CONFIG_UIO=y
CONFIG_UIO_PDRV_GENIRQ=y
```

*注意：如果文件中完全没有 `CONFIG_UIO` 相关行，直接在文件末尾添加这两行即可。*

### 4.2 更新配置依赖

保存文件后，执行以下命令以解析依赖关系并更新配置：

```sh
make ARCH=arm64 olddefconfig
```

这一步会根据手动添加的 `CONFIG_UIO=y` 自动检查并启用必要的依赖项。

### 4.3 验证配置

再次检查 `.config` 文件，确认配置已生效：

```sh
grep CONFIG_UIO .config
```

输出应显示：

```
CONFIG_UIO=y
CONFIG_UIO_PDRV_GENIRQ=y
```

### 4.4 重新编译内核

配置生效后，必须重新编译内核才能将 UIO 驱动包含进去：

```sh
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc)
```

### 4.5 使用新的 DTB 和内核运行

```sh
qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a57 \
  -m 2G \
  -kernel ~/workspace/linux-4.9.263/arch/arm64/boot/Image \
  -dtb ~/workspace/qemu_virt_machine/virt_custom.dtb \
  -initrd ~/workspace/busybox-1.33.1/initramfs.cpio.gz \
  -nographic \
  -append "console=ttyAMA0 rdinit=/linuxrc uio_pdrv_genirq.compat_id=generic-uio uio_pdrv_genirq.of_id=generic-uio"
```

> `uio_pdrv_genirq.compat_id=generic-uio` 是关键：告诉 `uio_pdrv_genirq` 把 compatible 为
> `generic-uio` 的 DT 节点绑定成 UIO 设备。

### 4.6 验证 UIO 设备

```sh
dmesg | grep -i uio
ls /dev/uio*
```

若看到 `/dev/uio0`，说明内核已把 fpga 节点导出成 UIO 设备。

## 5. 实现 libobject FPGA 驱动

### 5.1 类设计

libobject 里 FPGA 驱动分平台（`Uio_Fpga`）和 PCIe（`Uio_Pcie_Fpga`）两条继承链：

```
Obj → Uio ──────────────→ Uio_Fpga          （平台 UIO，DT 节点）
Obj → Uio → Uio_Pcie ───→ Uio_Pcie_Fpga     （PCIe 板卡 FPGA）
```

- **`Uio`**（基类）：`open`/`mmap`/`close`/`set_width`、寄存器读写、`enable/disable/register_irq`
  （基于 eventfd + io_worker 的异步中断）。
- **`Uio_Fpga`**（[`Uio_Fpga.h`](../src/include/libobject/board/hal/uio/Uio_Fpga.h:26)）：
  在 `Uio` 之上加 `open_device(fpga, dev_path)`（按 `/dev/uioX` 打开并 mmap），默认设备名 `"fpga"`
  （由 `device_name` 属性控制），复用 `Uio` 的寄存器/中断接口。
- **`Uio_Pcie_Fpga`**（[`Uio_Pcie_Fpga.h`](../src/include/libobject/board/hal/uio/Uio_Pcie_Fpga.h:27)）：
  继承 `Uio_Pcie` 的 `open_device(bdf 发现)` / `bind_uio(bar)` / 配置空间 / 寄存器 / 中断接口，
  作为 PCIe FPGA 基类，后续可扩展具体型号接口。

`Uio_Fpga` 对外接口（继承自 `Uio` 为主）：

```c
/* open_device：按设备名 "fpga" 解析出 /dev/uioX 再打开 + mmap */
int (*open_device)(Uio_Fpga *fpga, char *dev_path);
int (*set_width)(Uio_Fpga *fpga, int width);
int (*read_register)(Uio_Fpga *fpga, uint64_t offset, uint64_t *data);
int (*write_register)(Uio_Fpga *fpga, uint64_t offset, uint64_t data);
int (*read_registers)(Uio_Fpga *fpga, uint64_t offset, uint64_t *data, uint32_t len);
int (*write_registers)(Uio_Fpga *fpga, uint64_t offset, uint64_t *data, uint32_t len);
int (*enable_irq)(Uio_Fpga *fpga);
int (*disable_irq)(Uio_Fpga *fpga);
int (*register_irq)(Uio_Fpga *fpga, uio_irq_handler_t handler, void *opaque);
```

### 5.2 典型用法

```c
allocator_t *allocator = allocator_get_default_instance();
Uio_Fpga *fpga = object_new(allocator, "Uio_Fpga", NULL);
char dev_path[128] = {0};

uio_find_dev("fpga", dev_path, sizeof(dev_path));   /* /dev/uioX 路径解析 */
fpga->open_device(fpga, dev_path);                  /* 打开 + mmap */

fpga->write_register(fpga, 0x0, 0xDEADBEEF);        /* 寄存器写 */
fpga->read_register(fpga, 0x0, &val);               /* 寄存器读回 */

/* 异步中断：注册 handler，写 0xFF0 触发 SPI 70，io_worker 异步回调 */
fpga->enable_irq(fpga);
fpga->register_irq(fpga, my_irq_handler, fpga);
fpga->write_register(fpga, 0xFF0, 1);               /* 触发中断 */
/* ... 等待 handler 被调用（中断计数 uio->irq_count +1）... */
fpga->write_register(fpga, 0xFF0, 0);               /* 清除（高电平触发） */
fpga->disable_irq(fpga);
```

> 中断回调 `opaque` 是 io_worker 传入的 `Worker*`，通过 `((Worker*)opaque)->opaque` 取回
> `register_irq` 时传的 `fpga` 对象；中断计数由 `__irq_ev_callback` 存到 `uio->irq_count`。

### 5.3 编译 libobject（aarch64）

在 libobject 仓库根目录执行：

```bash
./devops.sh build --platform=linux --arch=aarch64
```

编译产物位于 `sysroot/linux/aarch64/bin/xtools`，动态库位于 `sysroot/linux/aarch64/lib/`。

### 5.4 部署代码到 QEMU（推荐：9p 共享目录）

由于代码会经常修改，推荐使用 **9p virtfs 共享目录**，宿主机与 guest 共享一个目录，改代码后只需
重新编译，guest 里直接看到新文件，**无需重启 QEMU**。

内核需支持 9p（`CONFIG_NET_9P=y`、`CONFIG_NET_9P_VIRTIO=y`、`CONFIG_9P_FS=y`）。启动 QEMU 时追加
`-virtfs` 参数，共享 libobject 的 `sysroot/linux/aarch64` 目录：

```sh
qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a57 \
  -m 2G \
  -kernel ~/workspace/linux-4.9.263/arch/arm64/boot/Image \
  -dtb ~/workspace/qemu_virt_machine/virt_custom.dtb \
  -initrd ~/workspace/busybox-1.33.1/initramfs.cpio.gz \
  -nographic \
  -virtfs local,path=/home/alan/workspace/libobject/sysroot/linux/aarch64,mount_tag=host0,security_model=none,id=host0 \
  -append "console=ttyAMA0 rdinit=/linuxrc uio_pdrv_genirq.compat_id=generic-uio uio_pdrv_genirq.of_id=generic-uio"
```

## 6. 测试

测试分为两部分：**手动测试**（用 shell 命令直接验证）和 **case 测试**（运行 libobject 的自动化
测试用例，位于 [`test_uio.c`](../tests/board/test_uio.c) 与 [`test_uio_fpga.c`](../tests/board/test_uio_fpga.c)）。

### 6.1 手动测试

#### 验证 UIO 设备

```sh
# 查看 UIO 设备
ls -l /dev/uio*
cat /sys/class/uio/uio0/name           # 期望: fpga
cat /sys/class/uio/uio0/maps/map0/addr # 期望: 0x0b000000
cat /sys/class/uio/uio0/maps/map0/size # 期望: 0x1000
```

#### 验证寄存器读写

```sh
# 用 devmem 读写寄存器（若 busybox 有 devmem）
devmem 0x0b000000 32 0x12345678
devmem 0x0b000000 32                  # 期望: 0x12345678
```

#### 验证中断

手动验证中断只需验证**第一次中断**（计数从 0 变 1），简单可靠：

```sh
# 1. 查看当前中断计数（初始应为 0）
cat /sys/class/uio/uio0/event

# 2. 触发中断（写 vfpga 中断控制寄存器 0xFF0 的 bit0=1）
devmem 0x0b000ff0 32 1

# 3. 再次查看中断计数，应 +1
cat /sys/class/uio/uio0/event         # 期望: 1

# 4. 清除中断（高电平触发需复位，写 0xFF0 = 0）
devmem 0x0b000ff0 32 0
```

> **说明**：
> - `/sys/class/uio/uio0/event` 只读当前中断计数，**不阻塞**，触发中断后计数 +1。
> - 中断为高电平触发（`interrupts = <0 70 4>`），触发后必须写 `0x0b000ff0` 为 0 清除，否则中断持续触发。
> - **多次中断递增验证**（计数 +2、+3 等）在 QEMU 中较复杂：`uio_pdrv_genirq` 在每次中断后自动屏蔽
>   中断，需读 `/dev/uio0` 重新使能，且读操作是阻塞的，时序难以控制。**推荐用 case 测试
>   `test_uio_fpga_irq` 验证多次中断**（自动触发 + 等待 + 验证计数，已确认可靠）。

### 6.2 case 测试

在 guest 里挂载 9p 共享目录后，用 `xtools mockery` 跑测试：

```sh
mkdir -p /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt
export LD_LIBRARY_PATH=/mnt/lib
```

#### 通用 UIO 测试（test_uio）

```sh
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_uio
```

测试通过时日志显示 `test suc, func_name = test_uio`，且 `read register[0x0] = 0xdeadbeef`。

#### FPGA 寄存器读写（test_uio_fpga_write_register / test_uio_fpga_write_registers）

```sh
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_uio_fpga_write_register
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_uio_fpga_write_registers
```

前者验证单个寄存器写读回环（`0xDEADBEEF`），后者验证批量 `read/write_registers`（len=4，地址 0x10）。

#### FPGA 异步中断（test_uio_fpga_irq，自动触发）

`test_uio_fpga_irq` 测试已实现**自动触发中断**：先使能中断，再写 `0xFF0` 触发 SPI 70 中断，然后
`wait` 轮询等待异步 handler 被调用（中断计数 +1）。运行测试即可，无需手动注入：

```sh
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_uio_fpga_irq
```

预期日志：

```
[INFO]-[enable_irq ok]
[INFO]-[register_irq ok]
[INFO]-[trigger irq ok (write 0xFF0 = 1)]
[INFO]-[async irq handled ok, irq_count:1]
[INFO]-[disable_irq ok]
[WIP]-[command suc, func_name = test_uio_fpga_irq, ...]
```

## 7. 常见问题

- **`ls /dev/uio*` 为空**：确认内核 `CONFIG_UIO_PDRV_GENIRQ=y` 且已重编；确认启动参数带
  `uio_pdrv_genirq.of_id=generic-uio`；确认 DT 节点 `compatible = "generic-uio"`。
- **中断计数不增长**：vfpga 中断是**高电平触发**，触发后必须写 `0x0b000ff0=0` 清除，否则无法再触发；
  先看 `dmesg | grep -i uio` 是否有中断上报。
- **`/sys/class/uio/uio0/name` 不是 fpga**：`name` 默认取 DT 节点名，若想固定为 `fpga`，可给节点加
  `uio_pdrv_genirq.of_name` 或按节点命名。
