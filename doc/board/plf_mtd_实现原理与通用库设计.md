# plf MTD 实现原理分析与通用库设计指南

## 1. 背景

MTD（Memory Technology Device）用于访问 **Flash 存储设备**（NAND/NOR/QSPI Flash）。在 spider 中，MTD 用于 FPGA 镜像存储、参数持久化存储、FPGA 元数据管理等场景。

本文档分析现有 MTD 使用方式，并为后续设计**通用的 Flash 访问库**提供参考。

## 2. 现有实现原理

### 2.1 总体架构

MTD 访问基于 **Linux MTD 子系统**（`/dev/mtd*` 或 `/dev/flash/*`），核心机制：

1. **打开设备**：`open(device, O_RDWR)` 或 `open(device, O_RDONLY)`
2. **获取设备信息**：`ioctl(fd, MEMGETINFO, &mtd)` 获取大小、擦除块大小
3. **擦除**：`ioctl(fd, MEMERASE, &e)` 按块擦除
4. **读写**：`lseek` + `read` / `write`
5. **挂载**：`mount("mtd:xxx", ..., "jffs2", ...)` 挂载为文件系统

### 2.2 核心 MTD 操作

| 操作 | 系统调用/ioctl | 说明 |
|------|---------------|------|
| 获取信息 | `ioctl(fd, MEMGETINFO, &mtd)` | 获取 `size`、`erasesize` |
| 擦除 | `ioctl(fd, MEMERASE, &e)` | 按块擦除（块对齐） |
| 读取 | `lseek` + `read` | 读取数据 |
| 写入 | `lseek` + `write` | 写入数据（需先擦除） |
| 挂载 | `mount("mtd:xxx", ..., "jffs2", ...)` | 挂载为文件系统 |

### 2.3 主要使用场景

| 场景 | 文件 | 用途 |
|------|------|------|
| Flash 拷贝 | [`libplf_util.c`](../common/meta-elsw-common/recipes-apps/plf-base-util/files/libplf_util.c:241) | 擦除+写入+校验 |
| FPGA 元数据 | [`ct11_early_init.c`](../spider_cpu/project-spec/meta-user/recipes-apps/ct11-early-init/files/ct11_early_init.c:87) | 擦除+挂载 jffs2 |
| 参数存储 | [`libplf_persistent_storage_block_device.c`](../common/meta-elsw-common/recipes-apps/plf-persistent-storage/files/libplf_persistent_storage_block_device.c:122) | 读取参数区+CRC 校验 |
| FPGA 镜像 | [`libplf_fpgaload.c`](../common/meta-elsw-common/recipes-apps/plf-fpgaload/files/libplf_fpgaload.c:288) | 读取+解压镜像 |

### 2.4 spider MTD 分区布局

spider 平台的 MTD 分区分为三类：**QSPI Flash 分区**（真实 Flash）、**phram 分区**（映射到 DDR 内存）、**mesh Flash 分区**（mesh CPU 的 Flash）。

#### 2.4.1 QSPI Flash 分区（PS 侧真实 Flash）

分区定义在 Petalinux 配置 [`config`](../spider_cpu/project-spec/configs/config:97) 中（`CONFIG_SUBSYSTEM_FLASH_PS7_QSPI_0_BANKLESS_PART*`），由 Petalinux 生成到设备树 `&flash0` 节点（见 [`system-include.dtsi`](../spider_cpu/project-spec/meta-user/recipes-bsp/device-tree/files/system-include.dtsi:58)）：

| 分区 | 大小 | 用途 |
|------|------|------|
| `spider_boot` | 16M | BOOT.BIN |
| `spider_hole` | 112M | 空洞（被删除，见 AR#57744） |
| `spider_kernel` | 20M | 内核 image.ub |
| `spider_golden_kernel` | 12M | 备用内核 |
| `spider_golden_fpga` | 4M | 备用 FPGA 镜像 |
| `spider_fpga` | 18M | FPGA 镜像 |
| `cpri_fpga` | 19M | CPRI FPGA 镜像 |
| `spider_parameters` | 1M | 参数持久化存储 |
| `spider_data` | 54M | 数据区（挂载为 jffs2） |

这些分区通过 Zynq PS 的 QSPI 控制器（`&qspi`）访问，**与 FPGA 无关**。

#### 2.4.2 phram 分区（映射到 DDR 内存）

`spider_meta` 不是真实 Flash，而是通过 **phram 驱动**把一段 DDR 物理内存当作 MTD 设备：

- 在 [`device-tree-generation_%.bbappend`](../spider_cpu/project-spec/meta-user/recipes-bsp/device-tree/device-tree-generation_%.bbappend:20) 中通过内核启动参数创建：
  ```
  phram.phram=spider_meta,80Mi,8Mi,64ki
  ```
  即：物理地址 `0x5000000`（80Mi），大小 8Mi，擦除块 64ki。
- 对应地在 [`system-include.dtsi`](../spider_cpu/project-spec/meta-user/recipes-bsp/device-tree/files/system-include.dtsi:10) 中把这段内存声明为 reserved-memory（`spider_meta@05000000`，`no-map`），防止内核正常使用。

`spider_meta` 用于存储 FPGA 元数据，挂载为 jffs2 到 `/mnt/spider_meta`，见 [`ct11_early_init.c`](../spider_cpu/project-spec/meta-user/recipes-apps/ct11-early-init/files/ct11_early_init.c:135)。

#### 2.4.3 mesh Flash 分区

mesh CPU 的 Flash 分区定义在 [`system-include.dtsi`](../spider_cpu/project-spec/meta-user/recipes-bsp/device-tree/files/system-include.dtsi:76)：

| 分区 | 大小 | 用途 |
|------|------|------|
| `mesh_boot` | 16M | mesh 启动 |
| `mesh_kernel` | 32M | mesh 内核 |
| `mesh_fpga` | 16M | mesh FPGA 镜像 |
| `mesh_data` | 256M | mesh 数据区 |

#### 2.4.4 分区访问方式汇总

| 分区 | 类型 | 访问方式 |
|------|------|---------|
| `spider_boot`/`spider_kernel`/`spider_fpga` 等 | QSPI Flash | `open + ioctl(MEMGETINFO/MEMERASE) + read/write` |
| `spider_parameters` | QSPI Flash | `plf_util_flashcp_page()` 写入，`open + lseek + read` 读取 |
| `spider_data` | QSPI Flash | 挂载为 jffs2 到 `/mnt/persistent_storage` |
| `spider_meta` | phram (DDR) | 挂载为 jffs2 到 `/mnt/spider_meta` |
| `mesh_*` | mesh Flash | 由 mesh CPU 使用 |

### 2.5 典型流程（Flash 拷贝）

[`plf_util_flashcp()`](../common/meta-elsw-common/recipes-apps/plf-base-util/files/libplf_util.c:241)：

```c
// 1. 打开设备
fd_d = open(deviceName, O_SYNC | O_RDWR);

// 2. 获取设备信息
ioctl(fd_d, MEMGETINFO, &mtd);

// 3. 计算需要擦除的块数
erase_count = (image_size + mtd.erasesize - 1) / mtd.erasesize;

// 4. 逐块擦除
for (i = 1; i <= erase_count; i++) {
    e.length = mtd.erasesize;
    e.start = (i-1) * mtd.erasesize;
    ioctl(fd_d, MEMERASE, &e);
}

// 5. 写入数据
plf_util_copy_fd(fd_d, fd_f, offset, image_size, true, ...);

// 6. 校验
plf_util_copy_fd(fd_d, fd_f, offset, image_size, false, ...);
```

## 3. 现有实现存在的问题

### 3.1 功能性问题

1. **MTD 操作分散在多个库**：Flash 拷贝在 `plf-base-util`，参数存储在 `plf-persistent-storage`，FPGA 镜像在 `plf-fpgaload`，元数据在 `ct11-early-init`。**没有统一的 MTD 访问库**，各库重复实现 `MEMGETINFO`/`MEMERASE`/读写逻辑。

2. **设备路径硬编码**：`/dev/flash/*` 路径分散在各库中硬编码，无法配置。

3. **无坏块处理**：NAND Flash 有坏块，但代码中未使用 `MEMGETBADBLOCK`/`MEMSETBADBLOCK` 处理坏块。

4. **无 OOB（Out-of-Band）处理**：NAND 的 OOB 区（ECC、坏块标记）未处理，依赖内核 MTD 驱动。

### 3.2 健壮性问题

5. **无并发保护**：MTD 操作（擦除+写入）不是原子的，多进程同时操作同一 Flash 会冲突。

6. **无超时机制**：擦除/写入可能耗时较长，无超时控制。

7. **错误处理不统一**：各库的错误处理方式不一致，返回值语义不同。

8. **`plf_util_flashcp_page()` 用栈缓冲区**：`readBuffer` 用 `malloc` 分配，但未检查 `malloc` 失败。

### 3.3 性能问题

9. **逐块擦除无批量优化**：`plf_util_flashcp()` 逐块擦除，未利用 MTD 的批量擦除能力。

10. **无写均衡（Wear Leveling）**：直接操作 MTD 无写均衡，频繁写入会加速 Flash 磨损。若需写均衡，应使用文件系统（如 jffs2/ubifs）。

## 4. 通用库设计建议

### 4.1 设计目标

设计一个**通用的 Flash 访问库**，抽象 MTD 操作，提供统一的擦除/读写/校验接口，供各业务模块复用。

### 4.2 分层架构

```
┌─────────────────────────────────────────────┐
│              应用层 (调用方)                  │
│   fpga_load / param_storage / meta_manage    │
└───────────────────┬─────────────────────────┘
                    │ 统一 API
┌───────────────────▼─────────────────────────┐
│           Flash 访问抽象层 (FAL)              │
│   - 设备管理 (open/close)                    │
│   - 擦除 (erase)                            │
│   - 读写 (read/write)                       │
│   - 校验 (verify)                           │
│   - 并发控制 (锁)                            │
└───────────────────┬─────────────────────────┘
                    │ 后端适配器
┌───────────────────▼─────────────────────────┐
│  Backend A: MTD   │  Backend B: 文件系统     │
│  (ioctl+read/write)│  (jffs2/ubifs mount)    │
└───────────────────┴─────────────────────────┘
```

### 4.3 核心抽象接口

```c
// Flash 设备操作接口
typedef struct plf_flash_ops {
    int  (*open)(plf_flash_t *flash, const char *device);
    int  (*close)(plf_flash_t *flash);
    int  (*get_info)(plf_flash_t *flash, plf_flash_info_t *info);
    int  (*erase)(plf_flash_t *flash, uint32_t offset, uint32_t size);
    int  (*read)(plf_flash_t *flash, uint32_t offset, void *buf, size_t size);
    int  (*write)(plf_flash_t *flash, uint32_t offset, const void *buf, size_t size);
    int  (*verify)(plf_flash_t *flash, uint32_t offset, const void *buf, size_t size);
} plf_flash_ops_t;

// Flash 设备信息
typedef struct plf_flash_info {
    uint32_t size;        // 总大小
    uint32_t erasesize;   // 擦除块大小
    uint32_t writesize;   // 最小写单元
    uint32_t oobsize;     // OOB 大小
    bool     has_badblocks; // 是否有坏块
} plf_flash_info_t;
```

### 4.4 配置结构体

```c
// Flash 设备配置
typedef struct plf_flash_config {
    const char *device;       // 设备路径，如 "/dev/flash/spider_meta"
    bool        use_fs;       // 是否使用文件系统（jffs2/ubifs）
    const char *mount_point;  // 挂载点（use_fs=true 时）
    const char *fs_type;      // 文件系统类型："jffs2" / "ubifs"
} plf_flash_config_t;
```

### 4.5 关键设计点

1. **统一擦除-写入-校验流程**：封装 `plf_flash_write()` 为"擦除+写入+校验"原子操作，避免各库重复实现。

2. **块对齐处理**：擦除必须块对齐，写入需处理跨块边界。库内部自动处理对齐，对上层透明。

3. **坏块处理**：NAND 场景支持坏块跳过（`MEMGETBADBLOCK`），提供坏块表管理。

4. **并发控制**：所有操作加锁，防止多进程冲突。

5. **文件系统模式**：对于需要写均衡的场景，支持挂载 jffs2/ubifs 后通过文件操作访问，而非直接操作 MTD。

6. **错误码统一**：定义统一的错误码（成功、设备不存在、擦除失败、写入失败、校验失败、坏块等）。

### 4.6 建议的通用库 API

```c
// 初始化/关闭
plf_flash_return_value_t plf_flash_init(const plf_flash_config_t *config);
plf_flash_return_value_t plf_flash_shutdown(void);

// 设备信息
plf_flash_return_value_t plf_flash_get_info(plf_flash_info_t *info);

// 擦除
plf_flash_return_value_t plf_flash_erase(uint32_t offset, uint32_t size);

// 读写
plf_flash_return_value_t plf_flash_read(uint32_t offset, void *buf, size_t size);
plf_flash_return_value_t plf_flash_write(uint32_t offset, const void *buf, size_t size);

// 擦除+写入+校验（原子）
plf_flash_return_value_t plf_flash_write_verify(uint32_t offset, const void *buf, size_t size);

// 校验
plf_flash_return_value_t plf_flash_verify(uint32_t offset, const void *buf, size_t size);

// 错误码
typedef enum plf_flash_return_value {
  LIBPLF_FLASH_OK            = 0x0,
  LIBPLF_FLASH_ERROR         = 0x1,
  LIBPLF_FLASH_ERASE_ERROR   = 0x2,
  LIBPLF_FLASH_WRITE_ERROR   = 0x3,
  LIBPLF_FLASH_READ_ERROR    = 0x4,
  LIBPLF_FLASH_VERIFY_ERROR  = 0x5,
  LIBPLF_FLASH_BAD_BLOCK     = 0x6,
  LIBPLF_FLASH_INVALID_ARG   = 0x7,
  LIBPLF_FLASH_NOT_INIT      = 0x8,
} plf_flash_return_value_t;
```

## 5. 实施建议

1. **先抽象接口**：定义统一的 `plf_flash_ops_t` 接口，确保不同后端（MTD、文件系统）可替换。
2. **实现 MTD 后端**：封装 `MEMGETINFO`/`MEMERASE`/读写逻辑为 MTD 后端。
3. **实现文件系统后端**：封装挂载 jffs2/ubifs 后通过文件操作访问。
4. **统一擦除-写入-校验**：提供 `plf_flash_write_verify()` 原子操作。
5. **块对齐处理**：库内部自动处理块对齐和跨块边界。
6. **并发控制**：所有操作加锁。
7. **充分测试**：覆盖擦除、读写、校验、坏块、并发竞争、跨块写入等场景。
