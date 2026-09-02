# PCIe 用户态驱动设计文档

## 1. 概述

基于 Linux **UIO（uio_pci_generic）** 实现用户态 PCIe 设备驱动（`Uio_Pcie` 类），
在复用通用 [`Uio`](../src/include/libobject/board/hal/uio/Uio.h) 类的基础上，
新增 PCIe 特有功能：设备发现、配置空间读写、绑定 uio_pci_generic 并 mmap BAR 访问寄存器。

### 1.1 为什么 PCIe 需要用户态接口（与 I2C/SPI 的区别）

| | I2C / SPI / MTD | PCIe |
|---|---|---|
| 访问模型 | 串行字节事务：发命令/地址 + 读/写数据 | **内存映射（BAR + MMIO）**，不是"总线字节事务" |
| 用户态抽象 | `open/read/write/transfer` 总线类 | **mmap BAR → 直接寄存器访问**（`Uio` 类语义） |
| 无内核驱动时 | 必须自己按协议发命令 | mmap 后按普通内存访问寄存器 |

因此 PCIe **不需要**像 I2c/Spi 那样的"总线事务类"，而是复用内存映射型访问
（`Uio`/`Uio_Fpga` 已是该范式），Uio_Pcie 只补 PCI 特有部分。

### 1.2 访问方式：基于 UIO

本设计基于内核标准 **UIO**（`uio_pci_generic`）把 PCIe 设备的 BAR 暴露为 `/dev/uioX`，
用户态 `open("/dev/uioX")` + `mmap` 直接寄存器访问，并可复用 `Uio` 的中断接口。

### 1.3 设计目标

- **复用 Uio**：寄存器读写/中断/映射全部继承 `Uio`，不重复实现。
- **不新增 Uio 接口方法**：通过修改既有 `open`（统一按 `/dev/uioX` 路径打开）和
  `mmap`（按 `uio->map_index` 选择 map）实现通用化，仅补一个工具函数 `uio_find_dev`。
- **公开 API 精简**：对外只留 `open_device/get_info/read_config/write_config/bind_uio`。
- **两类能力解耦**：设备发现 + 配置空间（纯 sysfs，无需 UIO）；寄存器访问（UIO 绑定）。

## 2. Uio 通用化（为复用做准备）

为了让 `Uio_Pcie` 复用 `Uio`，对 `Uio` 做了最小改动：

### 2.1 `open` 统一按 `/dev/uioX` 路径打开

原来 `Uio.open(uio, name)` 只按 UIO 设备名（`/sys/class/uio/uioX/name`）匹配。
PCIe 用 `uio_pci_generic` 绑定的设备**名字都叫 `"uio_pci_generic"`**，无法按名字区分，
必须通过 BDF 找到 `/dev/uioX` 后直接按路径打开。改动：

- [`Uio.open(uio, dev_path)`](../src/include/libobject/board/hal/uio/Uio.h:33)：统一按设备路径打开。
- 新增工具函数 [`uio_find_dev(name, dev_path)`](../src/include/libobject/board/hal/uio/Uio.h:77)：
  按名字解析成 `/dev/uioX`（Uio_Fpga/`test_uio` 先解析再 open，行为不变）。

### 2.2 `mmap` 按 map 序号选择并内部查找

原来 `Uio.mmap(uio)` 只映射 map0、offset=0。PCIe 设备有多个 BAR，每个 BAR 是独立
UIO map。改动：

- [`Uio.mmap(uio)`](../src/board/hal/uio/Uio.c:109) 读取 `uio->map_index` 指定的
  `maps/mapN/size` 作为映射大小。
- **mmap offset = `map_index × PAGE_SIZE`**（UIO 标准约定：内核
  [`uio_find_mem_index`](../linux-4.9.263/drivers/uio/uio.c:594) 用 `vm_pgoff` 即 map 索引查表，
  map0 → offset 0，mapN → offset N×PAGE_SIZE）。**不是 BAR 物理地址**。
- `uio->map_index` 默认 0，FPGA（map0）行为不变。

> **PCIe 例外**：`uio_pci_generic`（Linux 4.9）**不设置 `info.mem[]`**（见
> [`uio_pci_generic.c`](../linux-4.9.263/drivers/uio/uio_pci_generic.c:87)），所以
> `/sys/class/uio/uioX/maps/mapN/` 不存在，`Uio.mmap` 无法用于 PCIe。因此 `Uio_Pcie`
> 改为**从 `/sys/bus/pci/devices/<BDF>/resource` 读取 BAR 物理基址后 mmap
> `/dev/mem`**（见 4.3）；`Uio.mmap` 仍用于 Uio_Fpga 等普通 UIO 设备。

## 3. Uio_Pcie 类设计

### 3.1 类层次

```
Obj -> Uio -> Uio_Pcie
```

### 3.2 文件结构

| 文件 | 作用 |
|------|------|
| [`src/include/libobject/board/hal/uio/Uio_Pcie.h`](../src/include/libobject/board/hal/uio/Uio_Pcie.h) | Uio_Pcie 类头文件 |
| [`src/board/hal/uio/Uio_Pcie.c`](../src/board/hal/uio/Uio_Pcie.c) | Uio_Pcie 类实现 |
| [`tests/board/test_uio_pcie.c`](../tests/board/test_uio_pcie.c) | 测试用例 |

> 构建系统 `src/board/CMakeLists.txt` 使用 `file(GLOB_RECURSE)`，新增文件自动纳入编译。

### 3.3 公开接口

| 接口 | 语义 | 是否需 UIO |
|------|------|-----------|
| `open_device(vendor, device)` | 按 vendor/device ID 在 `/sys/bus/pci/devices` 查找并打开设备 | 否 |
| `get_info(&info)` | 读取 vendor/device/class/revision/irq/BAR 大小 | 否 |
| `read_config(offset, &data)` / `write_config(offset, data)` | 读写 PCI 配置空间 | 否 |
| `bind_uio(bar)` | 绑定 uio_pci_generic → `Uio.open("/dev/uioX")`（中断用）+ mmap `/dev/mem` 映射指定 BAR；**可多次调用映射多个 BAR 并存** | 是 |
| `read/write_register(s)` | Uio_Pcie 覆盖：地址高位是 BAR 序号、低位是 BAR 内偏移（`pcie_bar_addr(pcie,bar,off)`），可同时访问多 BAR | 是 |
| `enable/disable_irq`、`register_irq`、`set_width` | **继承 Uio**，映射后直接可用 | 是 |

> `open_bdf`（按 BDF 打开）收敛为内部函数（`__open_bdf`），是 `open_device` 的实现基础，
> 也是 `bind_uio` 定位 `/dev/uioX` 的依据，不对外暴露。

### 3.4 设备信息结构

```c
typedef struct pcie_dev_info {
    uint32_t vendor;      /* 厂商 ID */
    uint32_t device;      /* 设备 ID */
    uint32_t class;       /* 类别/子类/编程接口（0xMMNNPP） */
    uint32_t revision;    /* 修订版本 */
    int      irq;         /* 中断号，-1 表示无 */
    int      num_bars;    /* 有效 BAR 数量 */
    uint64_t bar_size[6]; /* 各 BAR 大小（字节），0 表示该 BAR 无效 */
} pcie_dev_info_t;
```

## 4. 核心实现原理

### 4.1 设备发现（sysfs）

`open_device` 扫描 `/sys/bus/pci/devices/`，读取各设备的 `vendor`/`device` 文件匹配；
匹配到后调用内部 `__open_bdf` 记录 BDF、并从 `resource` 文件解析各 BAR 大小。

### 4.2 配置空间（sysfs）

`read_config`/`write_config` 通过 `pread`/`pwrite` 操作
`/sys/bus/pci/devices/<BDF>/config`（标准 256 字节 + 扩展空间），无需 UIO。

### 4.3 绑定 UIO 并映射 BAR

```c
bind_uio(bar):
  1. 通过 /sys/bus/pci/devices/<BDF>/uio/uioX 找已绑定的 UIO 编号；
     未绑定则先写 "vendor device" 到 uio_pci_generic/new_id（id_table 为空，
     直接 bind 会 ENODEV），再写 BDF 到 bind。
  2. 确保 /dev/uioX 节点存在（无 devtmpfs 时按 /sys/class/uio/uioX/dev mknod）。
  3. 打开 /dev/uioX（仅首次；多 BAR 重复 bind 复用同一 fd 用于中断）。
  4. __map_bar(bar)：从 /sys/bus/pci/devices/<BDF>/resource（文本文件，始终存在）
     取 BAR 物理基址 bar_addr[bar]（与大小 bar_size[bar] 一起在 open 时解析），
     页对齐后 mmap /dev/mem（CONFIG_DEVMEM=y、CONFIG_STRICT_DEVMEM 未开）。
     结果存 pcie->bar_base[bar]（访问基址）/ pcie->map_base[bar]（页对齐，
     munmap 用），每个 BAR 独立映射互不覆盖 → 可多次 bind 映射多个 BAR 并存。
  5. 之后寄存器访问走 Uio_Pcie 覆盖的 read/write_register(s)：地址编码
     (bar << bar_shift) | off，bar_shift 由最大 BAR 大小动态确定。
```

> **多 BAR 地址编码（同 mono 的 RBS_FPGA_PCI_BAR_OFFSET）**：
> `read_register(pcie_bar_addr(pcie, bar, off))`，其中
> `pcie_bar_addr(pcie, bar, off) = (bar << pcie->bar_shift) | off`；
> Uio_Pcie 内部 `__decode_addr` 拆回 `bar = addr >> shift`、`off = addr & ((1<<shift)-1)`，
> 用 `bar_base[bar] + off` 访问。单 BAR 设备（如 edu）bar=0 时地址就是纯偏移，行为不变。

> **为什么不用 Uio.mmap / 不能 mmap resourceN**：`uio_pci_generic`（Linux 4.9）不设置
> `info.mem[]` → `/sys/class/uio/uioX/maps/mapN/` 不存在；且 **arm64 未定义
> `HAVE_PCI_MMAP`** → `/sys/bus/pci/devices/<BDF>/resourceN` 也不存在
> （`pci_create_resource_files()` 退化为空实现）。所以 PCIe 在 arm64 上只能通过
> **`/dev/mem`** 按 BAR 物理地址映射；UIO 绑定保留只是为了拿到**中断**
> （`/dev/uioX` 的 enable/register_irq）。

### 4.4 权限与前提

- 发现/配置空间：普通用户即可（读 sysfs）。
- `bind_uio` 寄存器访问：需要 **root**（写 bind 文件）+ 内核
  `CONFIG_UIO_PCI_GENERIC=y` + 一块**无内核驱动**的 PCIe 设备。

## 5. 使用示例

```c
#include <libobject/board/hal/uio/Uio_Pcie.h>

allocator_t *allocator = allocator_get_default_instance();
Uio_Pcie *pcie = object_new(allocator, "Uio_Pcie", NULL);

pcie->open_device(pcie, 0x1234, 0x11e8);  /* 按 vendor:device 发现（如 QEMU edu，PCI_VENDOR_ID_QEMU=0x1234） */
pcie->get_info(pcie, &info);               /* 设备信息 / BAR 大小 */

uint32_t cfg;
pcie->read_config(pcie, 0, &cfg);          /* 配置空间（无需 UIO） */

pcie->bind_uio(pcie, 0);                   /* 绑定 uio_pci_generic + /dev/mem 映射 BAR0 */
pcie->bind_uio(pcie, 2);                   /* 多 BAR：再映射 BAR2（互不影响） */
pcie->set_width(pcie, 32);

uint64_t val;
/* 地址编码：pcie_bar_addr(pcie, bar, off) = (bar<<bar_shift)|off */
pcie->read_register(pcie, pcie_bar_addr(pcie, 0, 0x00), &val);   /* 读 BAR0 寄存器 */
pcie->write_register(pcie, pcie_bar_addr(pcie, 0, 0x04), 0x12345678);
pcie->read_register(pcie, pcie_bar_addr(pcie, 2, 0x04), &val);   /* 读 BAR2 寄存器 */

object_destroy(pcie);                       /* 解除全部 BAR 映射 + close + 释放 */
```

## 6. 测试（QEMU 环境）

### 6.1 环境准备

1. **内核**：启用 `CONFIG_UIO_PCI_GENERIC=y`（连同 `CONFIG_UIO=y`），重新编译：
   ```sh
   cd ~/workspace/linux-4.9.263
   ./scripts/config --enable UIO_PCI_GENERIC
   make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc) Image
   ```
2. **QEMU**：`edu` 是现成的无驱动 PCIe 测试设备（vendor `0x1234`, device `0x11e8`，
   BAR0 寄存器空间），`-device edu` 挂到 virt 的 PCIe 总线。

### 6.2 启动命令

```bash
qemu-system-aarch64 -M virt -cpu cortex-a57 -m 2G \
  -kernel ~/workspace/linux-4.9.263/arch/arm64/boot/Image \
  -initrd ~/workspace/busybox-1.33.1/initramfs.cpio.gz -nographic \
  -device edu \
  -drive file=/home/alan/workspace/qemu_virt_machine/flash.img,if=pflash,index=1,format=raw \
  -virtfs local,path=/home/alan/workspace/libobject/sysroot/linux/aarch64,mount_tag=host0,security_model=none,id=host0 \
  -append "console=ttyAMA0 rdinit=/linuxrc uio_pdrv_genirq.of_id=generic-uio"
```

### 6.3 运行与预期

```sh
mkdir -p /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt
export LD_LIBRARY_PATH=/mnt/lib
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_uio_pcie
```

`test_uio_pcie` 用 edu 设备验证寄存器访问路径（核心逻辑在独立的 `test_uio_pcie_edu`）：
`open_device` → `bind_uio(0)` mmap BAR0 → 读 ID 寄存器（`0x00` 期望 `0x010000ed`）
→ 写/读 addr4 寄存器（`0x04`，`~val`）往返校验。

预期日志：
```
[INFO]-[edu device found, running register access test]
[INFO]-[uio mmap success, map:0, size:0x10000, base:0x...]
[INFO]-[edu id reg[0x00] = 0x010000ed]
[INFO]-[edu addr4 reg[0x04] write 0x12345678, read 0xedcba987]
[INFO]-[edu register read/write round-trip ok]
```

## 7. 与现有类的关系

```
Obj
 ├── Uio       通用用户态 UIO（open/mmap/寄存器/中断）——已通用化
 │    ├── Uio_Fpga    FPGA 专用（默认设备名 "fpga"）
 │    └── Uio_Pcie    PCIe 专用（本设计）
 │         └── Uio_Pcie_Fpga   PCIe 板卡上的 FPGA 基类（纯继承，暂不新增接口）
```

- `Uio_Fpga`、`Uio_Pcie` 都复用 `Uio`；`Uio_Pcie_Fpga` 直接复用 `Uio_Pcie`
  （open_device/bind_uio/寄存器/中断/配置空间），作为 PCIe FPGA 基类，后续可扩展。

## 8. 文件清单

| 文件 | 说明 |
|------|------|
| `src/include/libobject/board/hal/uio/Uio.h` / `src/board/hal/uio/Uio.c` | Uio 通用化（open 按路径、uio_find_dev、mmap 按 map_index） |
| `src/include/libobject/board/hal/uio/Uio_Pcie.h` | Uio_Pcie 类头文件 |
| `src/board/hal/uio/Uio_Pcie.c` | Uio_Pcie 类实现 |
| `src/include/libobject/board/hal/uio/Uio_Pcie_Fpga.h` / `src/board/hal/uio/Uio_Pcie_Fpga.c` | Uio_Pcie_Fpga 基类（纯继承） |
| `tests/board/test_uio_pcie.c` | 测试用例（edu 寄存器往返） |
| `doc/board/pcie设计文档.md` | 本文档 |
