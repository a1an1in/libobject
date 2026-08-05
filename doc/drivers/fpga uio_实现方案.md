# `plf_fpga_access_register` UIO 实现方案（Spider）

## 1. 概述

`plf_fpga_access_register` 是 `plf-hardware-access` 库中用于**通过 UIO（Userspace I/O）访问 FPGA 寄存器**的核心静态函数。在 Spider 平台上，该函数走的是 `PLF_SIGNALING_USE_LINX` 分支（无 `device` 参数版本）。

整体访问链路如下：

```
应用层 API (plf_fpga_read_register / plf_fpga_write_register ...)
        │
        ▼
plf_fpga_access_register(address, data, access_length, type)   ← 核心函数
        │
        ▼
libuio_helper (uio_open / uio_mmap / uio_getsize / uio_close)
        │
        ▼
Linux UIO 驱动 (uio_pdrv_genirq, compatible = "generic-uio")
        │
        ▼
设备树 (axi_2_ibus 节点, reg = <0x40000000 0x800000>)
        │
        ▼
FPGA AXI 寄存器空间
```

---

## 2. 关键文件与配置

| 文件 | 作用 |
|------|------|
| [`fpga.c`](spider_cpu/project-spec/meta-user/recipes-apps/plf-hardware-access-ct11/files/fpga.c) | 核心实现 |
| [`Makefile.lib`](spider_cpu/project-spec/meta-user/recipes-apps/plf-hardware-access-ct11/files/Makefile.lib) | 编译配置，定义 `PLF_SIGNALING_USE_LINX` |
| [`uio_helper.c`](common/meta-elsw-common/recipes-apps/libuio-helper/files/crl-libuio_helper_2.1.3/src/uio_helper.c) | UIO 封装库 |
| [`pl_spider.dtsi`](common/meta-elsw-common/recipes-apps/fpga-dt/fpga-dt/fpga_dt/dtsi/spider/common/pl_spider.dtsi) | `axi_2_ibus` 节点原始定义（含 `reg` 地址） |
| [`system-include.dtsi`](spider_cpu/project-spec/meta-user/recipes-bsp/device-tree/files/system-include.dtsi) | 覆盖 `axi_2_ibus` 的 `compatible` 为 `generic-uio` |
| [`libplf_hardware_access.h`](common/meta-elsw-common/recipes-apps/plf-hardware-access-api/files/libplf_hardware_access.h) | 类型与返回值定义 |

**关键配置确认**：
- [`Makefile.lib`](spider_cpu/project-spec/meta-user/recipes-apps/plf-hardware-access-ct11/files/Makefile.lib:12) 中 `CFLAGS += "-D PLF_SIGNALING_USE_LINX"`，因此编译时启用 LINX 分支。
- 内核启动参数需包含 `uio_pdrv_genirq.of_id=generic-uio`（见 [`device-tree-generation_%.bbappend`](spider_cpu/project-spec/meta-user/recipes-bsp/device-tree/device-tree-generation_%.bbappend:17)）。

---

## 3. 设备树地址空间（回答：为什么只有 compatible，地址在哪）

`axi_2_ibus` 的**地址空间（`reg`）并不在 `system-include.dtsi` 中定义**，而是来自 `fpga-dt` 包提供的原始节点定义。

### 3.1 原始定义（含地址）

[`pl_spider.dtsi`](common/meta-elsw-common/recipes-apps/fpga-dt/fpga-dt/fpga_dt/dtsi/spider/common/pl_spider.dtsi:472)：

```dts
axi_2_ibus: axi_2_ibus@40000000 {
    compatible = "xlnx,axi-2-ibus-1.0";
    reg = <0x40000000 0x800000>;   // 物理地址 0x40000000，大小 0x800000 (8MB)
};
```

### 3.2 覆盖定义（只改 compatible）

[`system-include.dtsi`](spider_cpu/project-spec/meta-user/recipes-bsp/device-tree/files/system-include.dtsi:142)：

```dts
&axi_2_ibus {
    compatible = "generic-uio";   // 仅覆盖 compatible，reg 保留原始值
};
```

`&axi_2_ibus` 是**设备树引用（reference）**，它只修改/追加属性，不会删除原始节点。因此最终编译后的节点为：

```dts
axi_2_ibus@40000000 {
    compatible = "generic-uio";          // 被覆盖
    reg = <0x40000000 0x800000>;         // 保留自原始定义
};
```

### 3.3 地址来源链路

```
fpga-dt 包 (pl_spider.dtsi)  ──提供──▶  ${STAGING_DATADIR}/fpga_dt/dtsi
        │
        ▼
device-tree-generation_%.bbappend
  DEPENDS += "fpga-dt"
  DEVICETREE_FLAGS += "-i ${STAGING_DATADIR}/fpga_dt/dtsi"   ← 编译时 include
        │
        ▼
system-include.dtsi 中 &axi_2_ibus 覆盖 compatible
        │
        ▼
最终 dtb：axi_2_ibus@40000000 { compatible="generic-uio"; reg=<0x40000000 0x800000>; }
```

> **注意**：仓库中的 [`pl.dtsi`](spider_cpu/project-spec/meta-user/recipes-bsp/device-tree/files/pl.dtsi) 是空文件，且 [`device-tree-generation_%.bbappend`](spider_cpu/project-spec/meta-user/recipes-bsp/device-tree/device-tree-generation_%.bbappend:32) 会删除自动生成的 `pl.dtsi`。真正的 FPGA 节点定义来自 `fpga-dt` 包（`pl_spider.dtsi`），通过 `-i` include 路径引入。

---

## 4. 数据结构

```c
struct fpga_uio_handle {
  uint32_t *base;        // mmap 后的虚拟地址基址（按 32 位字寻址）
  uint32_t size;         // UIO 映射区域大小（字节）
  UIO_HANDLE_ uio_handle; // libuio_helper 的句柄
};

static struct fpga_uio_handle uio_h;  // 全局单例
static uint8_t initialized = 0;       // 初始化标志
```

---

## 5. 核心函数实现（LINX 分支）

```c
static plf_hwa_return_value_t plf_fpga_access_register(
    uint32_t address, uint32_t *data, uint32_t *access_length,
    plf_hwa_fpga_access_type_t type)
{
  uint32_t volatile *reg;
  uint32_t i;

  // 1. 检查是否已初始化
  if (uio_h.uio_handle == NULL) {
    TRACE_INFO("Err: UIO is not initialized");
    return LIBPLF_HWA_INIT_ERROR;
  }

  // 2. 地址越界检查（address 为字节偏移）
  if (address >= uio_h.size) {
    printf("Err: Address out of range!\n");
    TRACE_SERIOUS("Err: Adress out of range");
    return LIBPLF_HWA_FPGA_ERROR;
  }

  // 3. 字节地址 → 字地址（除以 4），得到 32 位寄存器指针
  address /= 4;
  reg = uio_h.base + address;

  switch (type) {
  case LIBPLF_HWA_FPGA_READ: {
    // 4a. 清空目标缓冲区
    memset(data, 0x0, *access_length);

    // 5a. 防止读取超出 UIO 范围，截断长度
    if (address * 4 + *access_length > uio_h.size) {
      *access_length = uio_h.size - address * 4;
    }

    // 6a. 逐字读取（用 for 循环而非 memcpy，保证 4 字节对齐访问，
    //     且避免 volatile 内存的 memcpy 告警）
    for (i = 0; i < (*access_length) / 4; i++) {
      data[i] = reg[i];
    }
    break;
  }
  case LIBPLF_HWA_FPGA_WRITE: {
    // 4b. 写入单个 32 位寄存器
    *reg = *data;
    break;
  }
  }

  return LIBPLF_HWA_OK;
}
```

---

## 6. 初始化流程（`init_fpga`）

在调用 `plf_fpga_access_register` 之前，必须先通过 `init_fpga()` 完成 UIO 设备打开与内存映射：

```c
plf_hwa_return_value_t init_fpga(const char *uio_name)
{
  if (NULL == uio_h.uio_handle) {
    // 1. 打开 UIO 设备（通过 /sys/class/uio 按 name 匹配）
    uio_h.uio_handle = uio_open(uio_name);
    if (uio_h.uio_handle == UIO_OPEN_FAILED) {
      return LIBPLF_HWA_FPGA_UNKNOWN_DEVICE;
    }

    // 2. mmap 映射 FPGA 寄存器空间到用户态
    uio_h.base = uio_mmap(uio_h.uio_handle);
    if (uio_h.base == MAP_FAILED) {
      return LIBPLF_HWA_FPGA_MMAP_ERROR;
    }

    // 3. 获取映射区域大小（字节）
    uio_h.size = uio_getsize(uio_h.uio_handle);
    initialized = 1;
  }
  return LIBPLF_HWA_OK;
}
```

`init_fpga` 由 [`common.c`](spider_cpu/project-spec/meta-user/recipes-apps/plf-hardware-access-ct11/files/common.c:22) 中的 `plf_hwa_initialize_detailed()` 调用，传入的 `fpga_uio_name` 默认即 `"axi_2_ibus"`。

---

## 7. libuio_helper 底层实现

### 7.1 `uio_open(name)` — 打开 UIO 设备

[`uio_open()`](common/meta-elsw-common/recipes-apps/libuio-helper/files/crl-libuio_helper_2.1.3/src/uio_helper.c:216) 流程：

1. 遍历 `/sys/class/uio/uioX/name`，找到与 `name`（如 `"axi_2_ibus"`）匹配的 UIO 设备编号。
2. 打开 `/dev/uioX`（`O_RDWR | O_SYNC`）。
3. 读取 `/sys/class/uio/uioX/maps/map0/name` 等属性，记录映射信息。
4. 返回 `struct uio_handle *` 句柄。

### 7.2 `uio_mmap(handle)` — 内存映射

[`__uio_mmap()`](common/meta-elsw-common/recipes-apps/libuio-helper/files/crl-libuio_helper_2.1.3/src/uio_helper.c:334) 流程：

1. 从 `/sys/class/uio/uioX/maps/map0/addr`、`offset`、`size` 读取物理地址、偏移和大小。
2. 计算页对齐后的映射大小 `map_size`。
3. 调用 `mmap(map_addr, map_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)`。
4. 返回映射后的虚拟地址基址。

### 7.3 `uio_getsize(handle)` — 获取大小

[`uio_getsize()`](common/meta-elsw-common/recipes-apps/libuio-helper/files/crl-libuio_helper_2.1.3/src/uio_helper.c:716) 返回 `hdl->msize[0]`，即 map0 的映射大小（字节）。

### 7.4 `uio_close(handle)` — 关闭

[`uio_close()`](common/meta-elsw-common/recipes-apps/libuio-helper/files/crl-libuio_helper_2.1.3/src/uio_helper.c:300) 依次执行 `uio_unbind_irq`、`uio_munmap`、释放内存、`close(fd)`。

---

## 8. 返回值定义

| 返回值 | 值 | 含义 |
|--------|-----|------|
| `LIBPLF_HWA_OK` | 0x0 | 成功 |
| `LIBPLF_HWA_INIT_ERROR` | 0x1 | UIO 未初始化 |
| `LIBPLF_HWA_FPGA_ERROR` | 0x7 | 地址越界等错误 |
| `LIBPLF_HWA_FPGA_UNKNOWN_DEVICE` | 0x8 | 打开 UIO 设备失败 |
| `LIBPLF_HWA_FPGA_MMAP_ERROR` | 0x9 | mmap 失败 |

---

## 9. 实现要点与注意事项

1. **地址单位**：`address` 参数是**字节偏移**，内部除以 4 转为 32 位字地址，因此要求地址 4 字节对齐。
2. **volatile 访问**：`reg` 声明为 `uint32_t volatile *`，确保每次读写都直接访问硬件，不被编译器优化。
3. **批量读取**：读取用 `for` 循环逐字拷贝而非 `memcpy`，保证 4 字节对齐访问，并避免 volatile 告警。
4. **越界保护**：读取时若 `address*4 + access_length > size`，会截断 `access_length`，防止越界访问。
5. **单例句柄**：`uio_h` 为全局静态变量，整个进程共享一个 UIO 映射。
6. **初始化前置**：必须先调用 `init_fpga()`（经由 `plf_hwa_initialize_detailed()`），否则返回 `LIBPLF_HWA_INIT_ERROR`。
7. **线程安全**：`fpga.c` 未对 `uio_h` 加锁，多线程并发访问同一 UIO 映射时需由上层保证互斥。
8. **地址空间来源**：`axi_2_ibus` 的 `reg = <0x40000000 0x800000>` 来自 `fpga-dt` 包的 `pl_spider.dtsi`，`system-include.dtsi` 仅覆盖 `compatible`。

---

## 10. 调用示例

```c
#include "libplf_hardware_access.h"

// 初始化（打开 axi_2_ibus UIO 设备并 mmap）
plf_hwa_initialize_detailed(true, false, "axi_2_ibus");

// 读单个寄存器
uint32_t val = 0;
plf_fpga_read_register(0x1000, &val);   // 读取偏移 0x1000 处的寄存器

// 写单个寄存器
plf_fpga_write_register(0x1000, 0xDEADBEEF);

// 批量读
uint32_t buf[4];
uint32_t len = sizeof(buf);
plf_fpga_read_registers(0x2000, buf, &len);
```
