# plf SPI 标准用法（spidev）分析与重新设计指南

## 1. 背景

spider 中 SPI 总线有三种使用方式：

| 使用场景 | 访问方式 | 用途 |
|---------|---------|------|
| CPRI FPGA 加载 | **spidev 用户空间接口**（标准） | 给 CPRI FPGA 下载 bitstream |
| Mesh Flash 控制 | FPGA AXI SPI 寄存器（手动） | 手动控制 mesh flash |
| PLL / 温度传感器 | FPGA 寄存器间接控制 | 通过 FPGA 逻辑驱动 SPI 外设 |

其中 **spidev 用户空间接口**是相对标准、通用的用法，也是本设计文档的重点。它使用 Linux 内核的 **SPI 子系统**（`CONFIG_SPI_SPIDEV`）暴露的 `/dev/spidevX.Y` 字符设备，通过 ioctl 完成 SPI 传输。

## 2. 现有标准用法实现原理

### 2.1 总体架构

[`libplf_drv_fpgaload.c`](../common/meta-elsw-common/recipes-apps/plf-drv-fpgaload/files/libplf_drv_fpgaload.c:228) 基于 **Linux spidev 驱动**（`/dev/spidev2.0`）实现 SPI 访问：

1. **打开设备**：`open("/dev/spidev2.0", O_RDWR)`
2. **配置模式**：`ioctl(fd, SPI_IOC_WR_LSB_FIRST, &msbf)` 设置 MSB first
3. **传输数据**：构造 `struct spi_ioc_transfer`，通过 `ioctl(fd, SPI_IOC_MESSAGE(1), &tr)` 发送
4. **关闭设备**：`close(fd)`

### 2.2 对外 API

定义于 [`libplf_drv_fpgaload.h`](../common/meta-elsw-common/recipes-apps/plf-drv-fpgaload/files/libplf_drv_fpgaload.h:38)：

| 函数 | 语义 |
|------|------|
| `plf_drv_fpgaload_init_spi()` | 打开并配置 SPI 设备 |
| `plf_drv_fpgaload_shutdown_spi()` | 关闭 SPI 设备 |
| `plf_drv_fpgaload_send_data(buf, size)` | 通过 SPI 发送数据（含字节序转换） |
| `plf_drv_fpgaload_clear_status()` | 清除 SSPC 状态寄存器（FPGA 寄存器操作） |
| `plf_drv_fpgaload_set_program_b()` | 设置 FPGA 编程信号（FPGA 寄存器操作） |
| `plf_drv_fpgaload_get_init_b()` / `get_done()` | 读取 FPGA 状态（FPGA 寄存器操作） |

### 2.3 核心 SPI 传输实现

[`spi_transfer()`](../common/meta-elsw-common/recipes-apps/plf-drv-fpgaload/files/libplf_drv_fpgaload.c:228)：

```c
static bool spi_transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
  struct spi_ioc_transfer tr = {0};
  tr.tx_buf = (uintptr_t)tx;
  tr.rx_buf = (uintptr_t)rx;
  tr.len = len;
  tr.cs_change = 0;
  tr.delay_usecs = 0;
  int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  return ((size_t) ret == len);
}
```

### 2.4 初始化配置

[`plf_drv_fpgaload_init_spi()`](../common/meta-elsw-common/recipes-apps/plf-drv-fpgaload/files/libplf_drv_fpgaload.c:279)：

```c
spifd = open(SPI_DEV_NAME, O_RDWR);   // /dev/spidev2.0
uint8_t msbf = 0;                     // 0 = MSB first
ioctl(spifd, SPI_IOC_WR_LSB_FIRST, &msbf);
// 时钟速率等使用设备树默认值，未显式配置
```

### 2.5 字节序处理

[`swapBufU32()`](../common/meta-elsw-common/recipes-apps/plf-drv-fpgaload/files/libplf_drv_fpgaload.c:314)：FPGA 是大端（BE），CPU 是小端（LE），发送前用 `ntohl()` 对每个 32 位字做字节序交换。

## 3. 现有实现存在的问题

### 3.1 功能性问题

1. **设备路径硬编码**：`SPI_DEV_NAME` 固定为 `/dev/spidev2.0`，无法配置。若系统有多个 SPI 控制器或设备树变化，无法选择。

2. **只支持写（tx），不支持读（rx）**：`plf_drv_fpgaload_send_data()` 调用 `spi_transfer(spifd, buf, NULL, size)` 时 `rx` 为 NULL，只做单向写。SPI 是全双工协议，很多场景需要同时读写（如读 flash 状态、读传感器）。

3. **未配置 SPI 模式（Mode 0/1/2/3）**：只设置了 MSB first，未通过 `SPI_IOC_WR_MODE` 设置 CPOL/CPHA。依赖设备树默认值，若设备需要特定模式则无法满足。

4. **未配置最大速率**：注释说明"使用设备树默认值"，但未通过 `SPI_IOC_WR_MAX_SPEED_HZ` 显式设置，无法针对不同设备调整速率。

5. **未配置字长（bits per word）**：默认 8 位，若设备需要其他字长（如 16 位）则无法满足。

### 3.2 健壮性问题

6. **无并发保护**：`spifd` 是全局静态变量，`plf_drv_fpgaload_send_data()` 没有加锁。若多线程/多进程同时调用，SPI 传输会互相干扰。

7. **无错误恢复**：SPI 传输失败（如 NACK、超时）后没有重试机制，直接返回错误。

8. **`spifd` 生命周期管理不完善**：`init_spi()` 中若 `open` 成功但后续 ioctl 失败，会 `close` 并置 0，但 `shutdown_spi()` 与 `init_spi()` 的调用顺序依赖调用方，缺少统一的资源管理。

### 3.3 性能问题

9. **单次传输无批量优化**：`SPI_IOC_MESSAGE(1)` 每次只传一个 `spi_ioc_transfer`。SPI 支持一次 ioctl 提交多个 transfer（`SPI_IOC_MESSAGE(n)`），可实现"写命令+读数据"的原子组合，减少系统调用。

## 4. 重新设计时的注意事项

### 4.1 明确设计目标

- **保持 API 兼容**还是**重新设计 API**？
- 是否需要支持**多 SPI 设备**（当前仅 `/dev/spidev2.0`）？
- 是否需要支持**全双工读写**（当前仅写）？
- 是否需要支持**多 SPI 模式/速率/字长**配置？

### 4.2 设备与配置管理

1. **设备路径可配置**：将 SPI 设备路径作为参数或配置项，而非硬编码。
2. **完整配置 SPI 参数**：通过 ioctl 显式设置：
   - `SPI_IOC_WR_MODE`：SPI 模式（CPOL/CPHA）
   - `SPI_IOC_WR_MAX_SPEED_HZ`：最大时钟速率
   - `SPI_IOC_WR_BITS_PER_WORD`：字长
   - `SPI_IOC_WR_LSB_FIRST`：位序
3. **提供配置结构体**：封装 SPI 配置（模式、速率、字长、位序），便于不同设备复用。

### 4.3 全双工读写（核心）

SPI 是全双工协议，应支持同时读写。推荐封装一个通用的传输函数：

```c
// 通用 SPI 传输：同时发送 tx 并接收 rx
plf_spi_return_value_t plf_spi_transfer(const void *tx, void *rx, size_t len);

// 便捷封装：写命令 + 读数据（原子组合）
plf_spi_return_value_t plf_spi_write_read(const void *cmd, size_t cmd_len,
                                          void *data, size_t data_len);
```

### 4.4 使用 `SPI_IOC_MESSAGE(n)` 实现原子组合传输

现有实现用 `SPI_IOC_MESSAGE(1)` 单次传输。推荐使用 `SPI_IOC_MESSAGE(n)` 一次提交多个 `spi_ioc_transfer`，实现"写命令 + 读数据"的原子组合：

```c
struct spi_ioc_transfer tr[2];

// tr[0]: 写命令
tr[0].tx_buf = (uintptr_t)cmd;
tr[0].rx_buf = 0;
tr[0].len = cmd_len;

// tr[1]: 读数据（全双工，同时发 dummy 字节）
tr[1].tx_buf = 0;
tr[1].rx_buf = (uintptr_t)data;
tr[1].len = data_len;

int ret = ioctl(fd, SPI_IOC_MESSAGE(2), tr);
```

**优点**：
- 一次 ioctl 完成"写命令+读数据"，减少系统调用
- 保证组合传输的原子性（片选在整个传输期间保持有效）
- 支持 `cs_change`、`delay_usecs` 等精细时序控制

### 4.5 并发与线程安全

1. **加锁保护**：所有 SPI 传输操作必须加锁（`pthread_mutex` 或文件锁），防止多线程/多进程干扰。
2. **锁粒度**：SPI 传输是原子的，锁应覆盖整个传输过程（从片选拉低到拉高）。
3. **复用 fd**：初始化时打开一次设备，后续操作复用，避免每次 open/close。

### 4.6 错误处理与恢复

1. **区分错误类型**：传输失败、超时、设备不存在等应返回不同错误码。
2. **重试机制**：对瞬时错误（如总线忙）提供可配置的重试次数。
3. **错误日志**：统一记录 `errno` 和具体错误信息。

### 4.7 可移植性与解耦

1. **抽象 SPI 层**：将"打开设备、配置、传输、关闭"抽象为内部接口，便于适配不同平台（Linux spidev、FPGA AXI SPI、模拟 SPI）。
2. **与 FPGA 加载逻辑解耦**：现有 `libplf_drv_fpgaload` 把 SPI 传输和 FPGA 加载逻辑（SSPC 寄存器操作）混在一起。建议将 SPI 传输抽成独立的 SPI 库，FPGA 加载逻辑单独保留。
3. **头文件独立**：公共 SPI API 头文件与实现解耦，便于其他库复用。

## 5. 建议的新库 API 设计

```c
// SPI 配置结构体
typedef struct plf_spi_config {
    const char *device;        // 设备路径，如 "/dev/spidev2.0"
    uint8_t     mode;          // SPI 模式 (SPI_MODE_0/1/2/3)
    uint32_t    max_speed_hz;  // 最大时钟速率
    uint8_t     bits_per_word; // 字长，默认 8
    uint8_t     lsb_first;     // 0 = MSB first, 1 = LSB first
} plf_spi_config_t;

// 初始化/关闭
plf_spi_return_value_t plf_spi_init(const plf_spi_config_t *config);
plf_spi_return_value_t plf_spi_shutdown(void);

// 通用传输（全双工）
plf_spi_return_value_t plf_spi_transfer(const void *tx, void *rx, size_t len);

// 便捷封装
plf_spi_return_value_t plf_spi_write(const void *buf, size_t len);
plf_spi_return_value_t plf_spi_read(void *buf, size_t len);
plf_spi_return_value_t plf_spi_write_read(const void *cmd, size_t cmd_len,
                                          void *data, size_t data_len);

// 错误码
typedef enum plf_spi_return_value {
  LIBPLF_SPI_OK            = 0x0,
  LIBPLF_SPI_ERROR         = 0x1,
  LIBPLF_SPI_TRANSFER_ERROR = 0x2,
  LIBPLF_SPI_TIMEOUT       = 0x3,
  LIBPLF_SPI_INVALID_ARG   = 0x4,
  LIBPLF_SPI_NOT_INIT      = 0x5,
} plf_spi_return_value_t;
```

## 6. 实施建议

1. **先确认需求**：明确是否需要多设备、全双工、多模式等能力，避免过度设计。
2. **以 `SPI_IOC_MESSAGE(n)` 为核心**：所有传输基于多 transfer 组合实现，保证原子性。
3. **完整配置 SPI 参数**：通过 ioctl 显式设置模式、速率、字长、位序。
4. **加锁保护**：所有传输操作加锁，复用 fd。
5. **与 FPGA 加载逻辑解耦**：将 SPI 传输抽成独立库，便于复用。
6. **充分测试**：覆盖正常读写、全双工、多模式、并发竞争、错误恢复等场景。
