# 通用用户态 I2C 驱动设计

## 1. 概述

本模块在 `drivers` 下实现一个**通用用户态 I2C 驱动**，基于 Linux `i2c-dev` 驱动（`/dev/i2c-N`）实现用户空间 I2C 访问。设计参考 [`i2c_实现原理与重新设计指南.md`](i2c_实现原理与重新设计指南.md)，针对原 `plf-i2c` 实现的问题进行了重新设计。

### 1.1 设计目标

- **原子事务**：以 `I2C_RDWR` ioctl 为核心，单次调用提交多个 `i2c_msg`，保证"写寄存器地址 + 读数据"等操作不被其他线程/驱动打断。
- **线程安全**：复用 fd + 进程内 `pthread_mutex`，实现单进程多线程场景下的互斥。
- **多总线支持**：`open` 时指定 `bus_number`，拼接 `/dev/i2c-N`，不再硬编码 `/dev/i2c-0`。
- **错误分类**：区分 NACK、总线错误、超时、非法参数等错误码。
- **可扩展**：采用类继承模式，便于按具体 I2C 设备扩展。

### 1.2 类层次结构

```
Obj -> I2c
```

- [`I2c`](../src/include/libobject/drivers/i2c/I2c.h)：通用 I2C 驱动基类，实现基于 `i2c-dev` 的通用逻辑。后续可按具体 I2C 设备继承扩展。

## 2. 文件结构

| 文件 | 作用 |
|------|------|
| [`src/include/libobject/drivers/i2c/I2c.h`](../src/include/libobject/drivers/i2c/I2c.h) | I2c 基类头文件：接口、属性、错误码 |
| [`src/drivers/i2c/I2c.c`](../src/drivers/i2c/I2c.c) | I2c 基类实现 |
| [`tests/drivers/test_i2c.c`](../tests/drivers/test_i2c.c) | 测试用例 |

> 构建系统 [`src/drivers/CMakeLists.txt`](../src/drivers/CMakeLists.txt) 使用 `file(GLOB_RECURSE)`，新增 `.c`/`.h` 文件会自动纳入编译，无需修改 CMake。

## 3. API 设计

### 3.1 错误码

```c
typedef enum i2c_return_value {
    I2C_ERR_OK          = 0x0,   /* 成功 */
    I2C_ERR_ERROR       = 0x2,   /* 通用错误 */
    I2C_ERR_WRITE       = 0x3,   /* 写错误 */
    I2C_ERR_READ        = 0x4,   /* 读错误 */
    I2C_ERR_NACK        = 0x5,   /* 无应答 */
    I2C_ERR_TIMEOUT     = 0x6,   /* 超时 */
    I2C_ERR_BUS         = 0x7,   /* 总线错误 */
    I2C_ERR_INVALID_ARG = 0x8,   /* 非法参数 */
} i2c_return_value_t;
```

### 3.2 接口

| 接口 | 语义 |
|------|------|
| `open(i2c, bus_number)` | 打开 `/dev/i2c-N`，复用 fd，初始化互斥锁 |
| `close(i2c)` | 关闭设备，销毁互斥锁 |
| `transfer(i2c, slave_addr, msgs, nmsgs)` | 原子事务（核心），基于 `I2C_RDWR` ioctl |
| `write(i2c, slave_addr, reg, buf, size)` | 仅写（reg 为起始寄存器地址，buf 为数据） |
| `read(i2c, slave_addr, reg, write_buf, write_size, read_buf, read_size)` | 读（reg 为寄存器地址；reg==0xFF 时直接读） |
| `set_retry(i2c, retry)` | 设置瞬时错误重试次数 |
| `set_width(i2c, width)` | 设置寄存器地址宽度：8 或 16 位，默认 8 位 |

## 4. 核心实现原理

### 4.1 原子事务（`transfer`）

以 `I2C_RDWR` ioctl 为核心，一次提交多个 `i2c_msg`：

```c
struct i2c_rdwr_ioctl_data data;
data.msgs = msgs;
data.nmsgs = nmsgs;
ioctl(fd, I2C_RDWR, &data);   /* 单次调用，原子事务 */
```

**优点**：
- 真正的单总线事务，不会被其他驱动/进程打断。
- 支持一次事务中多个写/读段（如写寄存器地址 + 读数据）。
- 支持 `I2C_M_NOSTART`、`I2C_M_STOP` 等标志，灵活控制总线时序。

### 4.2 线程安全

- **复用 fd**：`open` 时打开一次设备，后续操作复用，避免每次 open/close 开销。
- **进程内互斥锁**：`pthread_mutex` 保护整个 `I2C_RDWR` 事务，保证单进程内多线程互斥。
- 锁在 `open` 时初始化，`close`/`deconstruct` 时销毁。

### 4.3 多总线支持

`open(i2c, bus_number)` 拼接 `/dev/i2c-%d`，支持选择任意 I2C 控制器。

### 4.4 地址校验

7 位 I2C 从机地址合法范围为 `0x08-0x77`，非法地址返回 `I2C_INVALID_ARG`。

### 4.5 错误处理与重试

- 根据 `errno` 映射错误码：`ENXIO`/`EREMOTEIO` → NACK，`ETIMEDOUT` → 超时，`EIO` → 总线错误。
- 对瞬时错误（`EAGAIN`/`EBUSY`）按 `retry` 次数重试。

## 5. 使用示例

```c
#include <libobject/drivers/i2c/I2c.h>

allocator_t *allocator = allocator_get_default_instance();
I2c *i2c = object_new(allocator, "I2c", NULL);

/* 打开总线 0 */
i2c->open(i2c, 0);

/* 写：从寄存器 0x00 开始写 2 个字节 */
uint8_t wbuf[2] = {0xAA, 0xBB};
i2c->write(i2c, 0x50, 0x00, wbuf, sizeof(wbuf));

/* 读指定寄存器：读寄存器 0x00 的值（原子事务） */
uint8_t rbuf[2] = {0};
i2c->read(i2c, 0x50, 0x00, NULL, 0, rbuf, sizeof(rbuf));

/* 直接读（reg == 0xFF，不写寄存器地址） */
i2c->read(i2c, 0x50, 0xFF, NULL, 0, rbuf, sizeof(rbuf));

object_destroy(i2c);
```

## 6. 测试

[`tests/drivers/test_i2c.c`](../tests/drivers/test_i2c.c) 使用 `REGISTER_TEST_CMD` 注册，覆盖 `read` / `write` 接口。

**运行前提**：需要真实 I2C 总线（`/dev/i2c-N`）或 QEMU 中模拟的 I2C 控制器。若指定总线不存在，`open` 失败，测试会失败。

可通过环境变量覆盖测试参数：
- `TEST_I2C_BUS`：总线号（默认 0）
- `TEST_I2C_ADDR`：从机地址（默认 0x50）

> **注意**：QEMU `virt` 机器当前设备树没有 I2C 控制器。若需在 QEMU 中测试，需额外给 QEMU 添加 I2C 模拟设备（类似 `vfpga` 方案），可作为后续独立任务。
