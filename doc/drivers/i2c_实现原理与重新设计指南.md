# plf-i2c 实现原理分析与重新设计指南

## 1. 现有实现原理

### 1.1 总体架构

现有 [`libplf_i2c.c`](../spider_cpu/project-spec/meta-user/recipes-apps/plf-i2c/files/libplf_i2c.c:1) 基于 **Linux i2c-dev 驱动**（`/dev/i2c-0`）实现用户空间 I2C 访问，核心机制是：

1. **打开设备**：`open("/dev/i2c-0", O_RDWR)`
2. **设置从机地址**：`ioctl(fd, I2C_SLAVE, i2cAddress)`
3. **读写数据**：`write(fd, buf, size)` / `read(fd, buf, size)`
4. **并发控制**：通过 `fcntl(fd, F_SETLKW, &lock)` 对设备文件加**文件锁**（`struct flock`），实现跨进程/线程互斥

### 1.2 对外 API

定义于 [`libplf_i2c.h`](../spider_cpu/project-spec/meta-user/recipes-apps/plf-i2c/files/libplf_i2c.h:20)：

| 函数 | 语义 |
|------|------|
| `plf_i2c_write_read(addr, wbuf, wsize, rbuf, rsize)` | 单次事务：先写后读（原子操作，受锁保护） |
| `plf_i2c_write(addr, wbuf, wsize)` | 仅写 |
| `plf_i2c_read(addr, rbuf, rsize)` | 仅读 |
| `plf_i2c_mask_register(addr, reg, mask, setclear)` | 读-改-写寄存器（单次事务） |
| `plf_i2c_enable_all()` | 配置 PCA9535 I/O 扩展器，打开 I2C hub 总线 |

### 1.3 并发控制机制（关键设计）

[`lock_i2c()`](../spider_cpu/project-spec/meta-user/recipes-apps/plf-i2c/files/libplf_i2c.c:244) 的实现：

```c
static int lock_i2c()
{
  int fd = open(I2C_DEV_NAME, O_RDWR);
  if (fd == -1) return -1;

  struct flock lock;
  memset(&lock, 0, sizeof(lock));
  lock.l_type = F_WRLCK;          // 写锁（排他锁）
  fcntl(fd, F_SETLKW, &lock);     // 阻塞直到获得锁
  return fd;
}
```

**核心思想**：每次操作都重新 `open()` 设备文件，然后对该文件描述符加 `F_WRLCK` 排他锁。由于 `fcntl` 文件锁是**进程级**的（同一进程内不同 fd 也会互斥），因此能同时保证：
- **跨进程互斥**：不同进程打开同一设备文件时，`F_SETLKW` 会阻塞等待
- **跨线程互斥**：同一进程内不同线程的 fd 也会因文件锁而互斥

`unlock_i2c()` 通过 `close(fd)` 释放锁（关闭文件会自动释放锁）。

### 1.4 事务原子性

- `plf_i2c_write_read()` 和 `plf_i2c_mask_register()` 在**同一个锁保护区间**内完成"写+读"或"读-改-写"，保证整个事务不被其他进程/线程打断。
- 这是设计的关键点：**I2C 的"写寄存器地址 + 读数据"必须作为一个原子事务**，否则其他线程可能在写地址后、读数据前插入操作，导致读到错误的数据。

## 2. 现有实现存在的问题

### 2.1 功能性问题

1. **`plf_i2c_write()` 和 `plf_i2c_read()` 不是原子事务**：头文件注释已明确说明"Not thread safe for use in a write-read-scenario"。如果调用方先 `write()` 再 `read()`（两次独立调用），中间可能被其他线程插入操作，导致读回错误数据。**应始终使用 `plf_i2c_write_read()`**。

2. **`I2C_SLAVE` 与 `I2C_SLAVE_FORCE`**：使用 `I2C_SLAVE` 时，如果目标地址已被内核驱动绑定（如设备树中声明的设备），`ioctl` 会失败。某些场景需要 `I2C_SLAVE_FORCE` 才能访问。

3. **`plf_i2c_enable_all()` 硬编码**：PCA9535 的地址、寄存器、配置值全部硬编码在 [`libplf_i2c.c`](../spider_cpu/project-spec/meta-user/recipes-apps/plf-i2c/files/libplf_i2c.c:18) 中，与具体硬件强耦合，不利于复用。

### 2.2 健壮性问题

4. **设备路径硬编码**：`I2C_DEV_NAME` 固定为 `/dev/i2c-0`，无法配置。若系统有多个 I2C 控制器，无法选择。

5. **无超时机制**：`F_SETLKW` 是阻塞锁，若其他进程持锁异常退出或长时间占用，调用方会无限阻塞。

6. **错误处理不完整**：`plf_i2c_write()` 中 `ioctl` 失败时打印 `ioctl_ret`（ioctl 返回值）而非 `errno`，日志信息不准确。

7. **`plf_i2c_mask_register()` 的读-改-写**：先 `write(fd, &reg, 1)` 再 `read(fd, &value, 1)`，依赖 i2c-dev 的"写后读"行为（写寄存器地址后，读操作会读取该寄存器）。这种方式依赖具体设备行为，不够通用。

### 2.3 性能问题

8. **每次操作都 open/close**：每次调用都 `open()` + `fcntl` 加锁 + 操作 + `close()`，系统调用开销较大。对于高频 I2C 访问场景，可考虑复用 fd + 更轻量的锁。

## 3. 重新设计时的注意事项

### 3.1 明确设计目标

在重新设计前，先明确：
- **保持 API 兼容**还是**重新设计 API**？
- 是否需要支持**多 I2C 总线**（当前仅 `/dev/i2c-0`）？
- 是否需要支持**I2C 事件/错误恢复**（如总线仲裁、NACK 处理）？
- 目标平台是否有多个 I2C 控制器？

### 3.2 并发与事务原子性（最重要）

这是 I2C 库设计的**核心**，必须重点考虑：

1. **原子事务**：所有"写地址+读数据"、"读-改-写"操作必须在**单一锁区间**内完成，绝不能拆成多次独立加锁的调用。
2. **锁粒度**：
   - **文件锁（fcntl）**：跨进程安全，但每次 open/close 开销大。
   - **进程内互斥锁（pthread_mutex）**：快，但仅限单进程。
   - **建议**：若库仅被单进程内的多线程使用，用 `pthread_mutex` + 复用 fd 更高效；若需跨进程，用文件锁或 `flock`。
3. **锁顺序**：若同时操作多个 I2C 总线，需定义一致的加锁顺序，避免死锁。

### 3.3 使用 `I2C_RDWR` ioctl 实现真正的原子事务

现有实现用 `write()` + `read()` 两次系统调用，虽然在同一锁区间内，但**并非真正的单总线事务**（两次调用之间总线可能被其他内核驱动占用）。

**推荐方案**：使用 `I2C_RDWR` ioctl 一次提交多个 `i2c_msg`，实现真正的原子事务：

```c
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

struct i2c_msg msgs[2];
struct i2c_rdwr_ioctl_data data;

// msg[0]: 写寄存器地址
msgs[0].addr = i2cAddress;
msgs[0].flags = 0;                    // 写
msgs[0].len = writeSize;
msgs[0].buf = writeBuffer;

// msg[1]: 读数据
msgs[1].addr = i2cAddress;
msgs[1].flags = I2C_M_RD;             // 读
msgs[1].len = readSize;
msgs[1].buf = readBuffer;

data.msgs = msgs;
data.nmsgs = 2;
ioctl(fd, I2C_RDWR, &data);           // 单次调用，原子事务
```

**优点**：
- 真正的单总线事务，不会被其他驱动/进程打断
- 支持一次事务中多个写/读段（如写寄存器地址 + 读数据）
- 支持 `I2C_M_NOSTART`、`I2C_M_STOP` 等标志，灵活控制总线时序

### 3.4 设备与地址管理

1. **多总线支持**：将总线号作为参数或配置项，而非硬编码 `/dev/i2c-0`。
2. **地址校验**：I2C 地址为 7 位（0x08-0x77），应校验输入合法性。
3. **`I2C_SLAVE` vs `I2C_SLAVE_FORCE`**：提供配置选项，决定是否强制访问已被内核绑定的地址。
4. **10 位地址**：如需支持 10 位地址，需设置 `I2C_TENBIT`。

### 3.5 错误处理与恢复

1. **区分错误类型**：NACK（无应答）、总线仲裁丢失、超时等应返回不同错误码。
2. **重试机制**：对瞬时错误（如总线忙）可提供可配置的重试次数。
3. **超时控制**：为阻塞锁和 I2C 操作设置超时，避免无限阻塞。
4. **错误日志**：统一记录 `errno` 和具体错误信息，便于排查。

### 3.6 性能优化

1. **复用 fd**：初始化时打开一次设备，后续操作复用，避免每次 open/close。
2. **批量操作**：支持一次事务读写多个寄存器（用 `I2C_RDWR` 多 msg）。
3. **减少锁竞争**：读操作可用共享锁（`F_RDLCK`），写操作用排他锁（`F_WRLCK`），提高并发度。

### 3.7 可移植性与解耦

1. **抽象总线层**：将"打开总线、加锁、事务、解锁"抽象为内部接口，便于适配不同平台（如 Linux i2c-dev、I2C over FPGA、模拟 I2C）。
2. **配置化**：将 PCA9535 等硬件相关配置（地址、寄存器、掩码）移到配置表或设备树，而非硬编码。
3. **头文件独立**：公共 API 头文件与实现解耦，便于其他库复用。

### 3.8 线程安全与生命周期

1. **初始化/关闭**：提供 `plf_i2c_init()` / `plf_i2c_shutdown()`，管理 fd 和锁资源的生命周期。
2. **线程安全**：所有公共 API 必须线程安全，内部用锁保护共享状态。
3. **资源释放**：确保异常路径（错误返回）也能正确释放锁和 fd，避免泄漏。

## 4. 建议的新库 API 设计

```c
// 初始化/关闭
plf_i2c_return_value_t plf_i2c_init(int bus_number);          // 指定总线号
plf_i2c_return_value_t plf_i2c_shutdown(void);

// 原子事务（推荐使用）
plf_i2c_return_value_t plf_i2c_transfer(int addr,
                                        struct i2c_msg *msgs, int nmsgs);

// 便捷封装（基于 transfer 实现）
plf_i2c_return_value_t plf_i2c_write_read(int addr,
                                          const void *wbuf, size_t wsize,
                                          void *rbuf, size_t rsize);
plf_i2c_return_value_t plf_i2c_write(int addr, const void *buf, size_t size);
plf_i2c_return_value_t plf_i2c_read(int addr, void *buf, size_t size);

// 读-改-写（原子）
plf_i2c_return_value_t plf_i2c_mask_register(int addr, uint8_t reg,
                                             uint8_t mask, bool setclear);

// 错误码扩展
typedef enum plf_i2c_return_value {
  LIBPLF_I2C_OK            = 0x0,
  LIBPLF_I2C_ERROR         = 0x2,
  LIBPLF_I2C_WRITE_ERROR   = 0x3,
  LIBPLF_I2C_READ_ERROR    = 0x4,
  LIBPLF_I2C_NACK_ERROR    = 0x5,   // 新增：无应答
  LIBPLF_I2C_TIMEOUT       = 0x6,   // 新增：超时
  LIBPLF_I2C_BUS_ERROR     = 0x7,   // 新增：总线错误
  LIBPLF_I2C_INVALID_ARG   = 0x8,   // 新增：非法参数
} plf_i2c_return_value_t;
```

## 5. 实施建议

1. **先确认需求**：明确是否需要多总线、跨进程、事件处理等能力，避免过度设计。
2. **以 `I2C_RDWR` 为核心**：所有事务操作基于 `I2C_RDWR` ioctl 实现，保证原子性。
3. **复用 fd + 合适的锁**：单进程场景用 `pthread_mutex`，跨进程场景用文件锁。
4. **配置化硬件参数**：将 PCA9535 等硬件配置移出代码，改为配置表。
5. **充分测试**：覆盖正常读写、NACK、总线忙、并发竞争、异常恢复等场景。
