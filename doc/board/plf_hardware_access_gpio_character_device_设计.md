# plf-hardware-access GPIO 迁移到 Character Device 接口设计

## 1. 背景与动机

### 1.1 现状

当前 [`gpio.c`](../spider_cpu/project-spec/meta-user/recipes-apps/plf-hardware-access-ct11/files/gpio.c:1) 中的 GPIO 实现基于 **Linux sysfs GPIO 接口**（`/sys/class/gpio/`），通过读写 sysfs 文件来操作 GPIO：

- `/sys/class/gpio/export` / `/sys/class/gpio/unexport` — 导出/取消导出引脚
- `/sys/class/gpio/gpioN/direction` — 配置方向
- `/sys/class/gpio/gpioN/value` — 读写电平
- `/sys/class/gpio/gpiochipN` — 查询 GPIO 控制器基地址

### 1.2 问题

1. **sysfs 接口已被内核标记为 deprecated（弃用）**，官方推荐使用 character device 接口（`/dev/gpiochipN` + `libgpiod`）。
2. 每次操作都执行 export/unexport，存在性能开销，且引脚被占用时 export 会失败。
3. sysfs 接口不支持事件（中断）监听、不支持批量操作、不支持更细粒度的配置（如上下拉、驱动强度、活动电平）。
4. 现有实现存在 bug：`getPinConfig()` 用写模式打开 `direction` 文件却用 `fgets` 读取，导致读取方向配置功能失效。

### 1.3 目标

将 GPIO 访问从 sysfs 接口迁移到 **character device 接口（libgpiod）**，同时**保持对外 API 不变**，即 [`libplf_hardware_access.h`](../common/meta-elsw-common/recipes-apps/plf-hardware-access-api/files/libplf_hardware_access.h:302) 中声明的 GPIO 函数签名与语义完全兼容。

## 2. 现有 API 与语义

需要保持兼容的对外接口（定义于 [`libplf_hardware_access.h`](../common/meta-elsw-common/recipes-apps/plf-hardware-access-api/files/libplf_hardware_access.h:302)）：

| 函数 | 语义 |
|------|------|
| `plf_hwa_gpio_set(bank, pin, state)` | 将指定引脚设为高/低电平 |
| `plf_hwa_gpio_get(bank, pin, state*)` | 读取指定引脚当前电平 |
| `plf_hwa_gpio_set_config(bank, pin, config)` | 将指定引脚配置为输入/输出 |
| `plf_hwa_gpio_get_config(bank, pin, config*)` | 读取指定引脚当前方向配置 |

相关类型定义：

```c
typedef enum plf_hwa_gpio_bank {
  LIBPLF_GPIO_MIO  = 0x0,   // MIO 引脚 0-53
  LIBPLF_GPIO_EMIO = 0x1,   // EMIO 引脚 0-63
  LIBPLF_GPIO_BANK_INVALID
} plf_hwa_gpio_bank_t;

typedef enum plf_hwa_gpio_state {
  LIBPLF_GPIO_LOW  = 0x0,
  LIBPLF_GPIO_HIGH = 0x1,
  LIBPLF_GPIO_STATE_INVALID
} plf_hwa_gpio_state_t;

typedef enum plf_hwa_gpio_type {
  LIBPLF_GPIO_IN  = 0x0,
  LIBPLF_GPIO_OUT = 0x1,
  LIBPLF_GPIO_TYPE_INVALID
} plf_hwa_gpio_type_t;
```

## 3. 技术方案选型

### 3.1 方案对比

| 方案 | 说明 | 优点 | 缺点 |
|------|------|------|------|
| **A. libgpiod（推荐）** | 使用 `libgpiod` 库访问 `/dev/gpiochipN` | 官方推荐、API 稳定、支持事件/批量/细粒度配置、线程安全 | 需新增依赖 `libgpiod` |
| **B. 直接 ioctl** | 直接对 `/dev/gpiochipN` 发起 `GPIO_GET_LINEHANDLE_IOCTL` 等 ioctl | 无额外依赖 | 需自行封装大量 ioctl 结构体，易出错、维护成本高 |
| **C. 保持 sysfs** | 维持现状 | 无需改动 | 接口已弃用、存在 bug、性能差 |

**结论：采用方案 A（libgpiod）**。libgpiod 是对 character device ioctl 的官方封装，API 简洁、稳定，且与现有 sysfs 实现相比能显著提升健壮性和可维护性。

### 3.2 libgpiod 版本选择

- 优先使用 **libgpiod v1.x**（`gpiod_line_request_*` 系列 API），因为其 API 与现有 sysfs 语义映射最直接，且被广泛支持。
- 若目标平台内核/发行版已提供 **libgpiod v2.x**（`gpiod_line_request_config` 系列 API），也可采用 v2.x，但需注意 API 差异（见第 6 节）。

## 4. 总体架构设计

```
┌─────────────────────────────────────────────────────────────┐
│                    应用层 (调用方)                            │
│        plf_hwa_gpio_set / get / set_config / get_config      │
└───────────────────────────┬─────────────────────────────────┘
                            │ 对外 API 不变
┌───────────────────────────▼─────────────────────────────────┐
│              plf-hardware-access-ct11 (gpio.c)               │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐    │
│  │              GPIO 抽象层 (本设计新增)                  │    │
│  │  - 引脚编号解析 (bank/pin -> 全局 line offset)         │    │
│  │  - chip 打开/关闭管理                                 │    │
│  │  - 线程安全 (互斥锁)                                  │    │
│  └──────────────────────────┬───────────────────────────┘    │
│                             │ libgpiod API                   │
│  ┌──────────────────────────▼───────────────────────────┐    │
│  │              libgpiod (gpiod_*)                       │    │
│  └──────────────────────────┬───────────────────────────┘    │
└─────────────────────────────┼───────────────────────────────┘
                              │ ioctl
                    ┌─────────▼─────────┐
                    │  /dev/gpiochipN   │
                    └─────────┬─────────┘
                              │
                    ┌─────────▼─────────┐
                    │  内核 GPIO 子系统  │
                    │  (gpiolib + 驱动)  │
                    └───────────────────┘
```

## 5. 详细设计

### 5.1 引脚编号映射

与 sysfs 实现一致，Zynq SoC 的 GPIO 分为 MIO（0-53）和 EMIO（0-63）两个 bank。在 character device 接口中，每个 `/dev/gpiochipN` 对应一个 GPIO 控制器，其内部 line 从 0 开始编号。

**关键差异**：sysfs 中 MIO 和 EMIO 可能被映射到同一个 gpiochip（通过 `gpiobase + 54` 偏移区分），而 character device 接口中 MIO 和 EMIO 可能对应**不同的 gpiochip**（例如 `gpiochip0` 为 MIO，`gpiochip1` 为 EMIO），也可能在同一个 chip 内。

因此需要**动态探测**每个 chip 的 `label` 和 `ngpio`，确定 MIO/EMIO 分别落在哪个 chip 及 line 偏移。

### 5.2 数据结构

```c
// gpio.c 内部新增
typedef struct plf_hwa_gpio_chip_s {
    struct gpiod_chip *chip;      // libgpiod chip 句柄
    char              *name;      // chip 名称，如 "gpiochip0"
    char              *label;     // chip label，如 "zynq_gpio"
    unsigned int       ngpio;     // 该 chip 的 line 数量
    int                base;      // 该 chip 的全局基地址（用于兼容/调试）
} plf_hwa_gpio_chip_t;

// 全局状态
static plf_hwa_gpio_chip_t g_mio_chip;   // MIO bank 对应的 chip
static plf_hwa_gpio_chip_t g_emio_chip;  // EMIO bank 对应的 chip
static pthread_mutex_t     g_gpio_lock;  // 线程安全互斥锁
static bool                g_gpio_initialized = false;
```

### 5.3 初始化与探测

在 `plf_hwa_initialize_detailed()` 中（或新增内部 `gpio_init()`）完成 chip 探测：

```c
static int gpio_init(void)
{
    // 遍历 /dev/gpiochipN
    for (int i = 0; ; i++) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/gpiochip%d", i);
        struct gpiod_chip *chip = gpiod_chip_open_by_name(path);
        if (!chip) break;  // 没有更多 chip

        const char *label = gpiod_chip_label(chip);
        unsigned int ngpio = gpiod_chip_num_lines(chip);

        // 根据 label 判断是 MIO 还是 EMIO
        // 例如 label 含 "zynq_gpio" 且 ngpio >= 118 时，
        // line 0-53 为 MIO，line 54-117 为 EMIO
        // 若 MIO/EMIO 分属不同 chip，则分别记录
        ...
    }
    return 0;
}
```

**探测策略**（需根据实际设备树/驱动确认，二选一）：

1. **单一 chip 模式**：若 MIO 和 EMIO 在同一个 chip（`ngpio >= 118`），则：
   - MIO line offset = `pin`（0-53）
   - EMIO line offset = `pin + 54`（54-117）
2. **双 chip 模式**：若 MIO 和 EMIO 分属两个 chip，则分别记录 `g_mio_chip` 和 `g_emio_chip`，line offset 均为 `pin`。

> 建议在初始化时打印探测结果（chip 名称、label、ngpio），便于调试和确认实际布局。

### 5.4 对外 API 实现

#### 5.4.1 `plf_hwa_gpio_set()`

```c
plf_hwa_return_value_t plf_hwa_gpio_set(plf_hwa_gpio_bank_t bank, int pin,
                                        plf_hwa_gpio_state_t state)
{
    if (!verifyBankPinCombo(bank, pin))
        return LIBPLF_HWA_GPIO_BAD_INPUT_ERROR;

    pthread_mutex_lock(&g_gpio_lock);

    struct gpiod_line *line = get_line(bank, pin);
    if (!line) {
        pthread_mutex_unlock(&g_gpio_lock);
        return LIBPLF_HWA_GPIO_ACCESS_ERROR;
    }

    // 请求 line 为输出，并设置初始电平
    int req = gpiod_line_request_output(line, "plf-hwa",
                                        (state == LIBPLF_GPIO_HIGH) ? 1 : 0);
    if (req < 0) {
        pthread_mutex_unlock(&g_gpio_lock);
        return LIBPLF_HWA_GPIO_ACCESS_ERROR;
    }

    pthread_mutex_unlock(&g_gpio_lock);
    return LIBPLF_HWA_OK;
}
```

#### 5.4.2 `plf_hwa_gpio_get()`

```c
plf_hwa_return_value_t plf_hwa_gpio_get(plf_hwa_gpio_bank_t bank, int pin,
                                        plf_hwa_gpio_state_t* state)
{
    if (!verifyBankPinCombo(bank, pin))
        return LIBPLF_HWA_GPIO_BAD_INPUT_ERROR;

    pthread_mutex_lock(&g_gpio_lock);

    struct gpiod_line *line = get_line(bank, pin);
    if (!line) {
        pthread_mutex_unlock(&g_gpio_lock);
        return LIBPLF_HWA_GPIO_ACCESS_ERROR;
    }

    // 请求 line 为输入
    if (gpiod_line_request_input(line, "plf-hwa") < 0) {
        pthread_mutex_unlock(&g_gpio_lock);
        return LIBPLF_HWA_GPIO_ACCESS_ERROR;
    }

    int val = gpiod_line_get_value(line);
    pthread_mutex_unlock(&g_gpio_lock);

    if (val < 0)
        return LIBPLF_HWA_GPIO_ACCESS_ERROR;

    *state = (val == 1) ? LIBPLF_GPIO_HIGH : LIBPLF_GPIO_LOW;
    return LIBPLF_HWA_OK;
}
```

#### 5.4.3 `plf_hwa_gpio_set_config()`

```c
plf_hwa_return_value_t plf_hwa_gpio_set_config(plf_hwa_gpio_bank_t bank, int pin,
                                               plf_hwa_gpio_type_t config)
{
    if (!verifyBankPinCombo(bank, pin))
        return LIBPLF_HWA_GPIO_BAD_INPUT_ERROR;

    pthread_mutex_lock(&g_gpio_lock);

    struct gpiod_line *line = get_line(bank, pin);
    if (!line) {
        pthread_mutex_unlock(&g_gpio_lock);
        return LIBPLF_HWA_GPIO_ACCESS_ERROR;
    }

    int req;
    if (config == LIBPLF_GPIO_IN)
        req = gpiod_line_request_input(line, "plf-hwa");
    else
        req = gpiod_line_request_output(line, "plf-hwa", 0);

    pthread_mutex_unlock(&g_gpio_lock);

    return (req == 0) ? LIBPLF_HWA_OK : LIBPLF_HWA_GPIO_ACCESS_ERROR;
}
```

#### 5.4.4 `plf_hwa_gpio_get_config()`

```c
plf_hwa_return_value_t plf_hwa_gpio_get_config(plf_hwa_gpio_bank_t bank, int pin,
                                               plf_hwa_gpio_type_t* config)
{
    if (!verifyBankPinCombo(bank, pin))
        return LIBPLF_HWA_GPIO_BAD_INPUT_ERROR;

    pthread_mutex_lock(&g_gpio_lock);

    struct gpiod_line *line = get_line(bank, pin);
    if (!line) {
        pthread_mutex_unlock(&g_gpio_lock);
        return LIBPLF_HWA_GPIO_ACCESS_ERROR;
    }

    // 读取 line 当前方向配置
    int dir = gpiod_line_direction(line);
    pthread_mutex_unlock(&g_gpio_lock);

    if (dir == GPIOD_LINE_DIRECTION_INPUT)
        *config = LIBPLF_GPIO_IN;
    else if (dir == GPIOD_LINE_DIRECTION_OUTPUT)
        *config = LIBPLF_GPIO_OUT;
    else
        return LIBPLF_HWA_GPIO_ACCESS_ERROR;

    return LIBPLF_HWA_OK;
}
```

### 5.5 辅助函数

```c
// 校验 bank/pin 组合（与现有实现一致）
static int verifyBankPinCombo(plf_hwa_gpio_bank_t bank, int pin)
{
    if (bank == LIBPLF_GPIO_MIO)
        return (pin >= 0 && pin <= 53);
    if (bank == LIBPLF_GPIO_EMIO)
        return (pin >= 0 && pin <= 63);
    return 0;
}

// 根据 bank/pin 获取对应的 gpiod_line
static struct gpiod_line *get_line(plf_hwa_gpio_bank_t bank, int pin)
{
    if (!g_gpio_initialized && gpio_init() < 0)
        return NULL;

    plf_hwa_gpio_chip_t *chip = (bank == LIBPLF_GPIO_MIO) ? &g_mio_chip : &g_emio_chip;
    if (!chip->chip)
        return NULL;

    unsigned int offset = pin;
    // 单一 chip 模式下 EMIO 需要 +54 偏移
    if (bank == LIBPLF_GPIO_EMIO && g_single_chip_mode)
        offset += 54;

    return gpiod_chip_get_line(chip->chip, offset);
}
```

### 5.6 线程安全

- 使用 `pthread_mutex_t g_gpio_lock` 保护所有 GPIO 操作，与库中其他外设（I2C 等）的并发处理方式保持一致。
- 由于每次操作都通过 `gpiod_line_request_*` 重新请求 line，天然避免了跨线程的 line 状态冲突。

### 5.7 错误处理与返回值映射

| libgpiod 返回 | 映射到 plf_hwa 返回值 |
|---------------|----------------------|
| 成功 (0 / 非负) | `LIBPLF_HWA_OK` |
| line 请求失败 | `LIBPLF_HWA_GPIO_ACCESS_ERROR` |
| 非法 bank/pin | `LIBPLF_HWA_GPIO_BAD_INPUT_ERROR` |
| chip 打开失败 | `LIBPLF_HWA_GPIO_ACCESS_ERROR` |

## 6. libgpiod v1.x 与 v2.x API 差异

若目标平台使用 libgpiod v2.x，主要 API 差异如下：

| 功能 | v1.x | v2.x |
|------|------|------|
| 打开 chip | `gpiod_chip_open_by_name()` | `gpiod_chip_open()` |
| 获取 line | `gpiod_chip_get_line()` | `gpiod_chip_request_lines()` |
| 请求输出 | `gpiod_line_request_output()` | `gpiod_line_request_config()` + `gpiod_line_request_set_value()` |
| 读取方向 | `gpiod_line_direction()` | `gpiod_line_config_get_direction()` |
| 读取电平 | `gpiod_line_get_value()` | `gpiod_line_request_get_value()` |

> 设计上建议在 `gpio.c` 内部封装一层薄适配层，屏蔽 v1/v2 API 差异，便于在不同平台间移植。

## 7. 构建与依赖

### 7.1 新增依赖

在 [`Makefile.lib`](../spider_cpu/project-spec/meta-user/recipes-apps/plf-hardware-access-ct11/files/Makefile.lib:1) 中新增：

```makefile
LDLIBS += -lgpiod
```

### 7.2 内核配置

确保内核开启 character device GPIO 接口：

```
CONFIG_GPIO_CDEV=y
```

（`/dev/gpiochipN` 节点由 `CONFIG_GPIO_CDEV` 提供。）

### 7.3 设备树

确认设备树中 Zynq GPIO 节点（`xilinx_gpio`）已正确配置，且 MIO/EMIO 的 chip 布局符合第 5.3 节的探测假设。

## 8. 测试计划

### 8.1 单元/功能测试

1. **MIO 引脚读写**：对 MIO bank 的引脚执行 set/get，验证电平正确。
2. **EMIO 引脚读写**：对 EMIO bank 的引脚执行 set/get，验证电平正确。
3. **方向配置**：对引脚执行 set_config(IN/OUT) 后，用 get_config 验证方向正确。
4. **非法参数**：传入越界 pin 或非法 bank，验证返回 `LIBPLF_HWA_GPIO_BAD_INPUT_ERROR`。
5. **并发测试**：多线程同时操作不同引脚，验证无数据竞争、结果正确。

### 8.2 回归测试

- 使用现有依赖 `plf_hwa_gpio_*` 的应用（如 status、rrzcmd 等）进行回归，确保对外行为不变。

### 8.3 兼容性验证

- 在目标硬件上确认 `/dev/gpiochipN` 存在且可访问。
- 打印 chip 探测结果，确认 MIO/EMIO 布局与设计假设一致。

## 9. 风险与注意事项

1. **chip 布局不确定性**：MIO/EMIO 是否在同一 chip 取决于设备树和驱动，需在目标硬件上实测确认，必要时调整探测逻辑。
2. **line 占用冲突**：若某 line 已被内核或其他进程占用（如被设备树声明为固定功能），`gpiod_line_request_*` 会失败，需返回明确的错误码。
3. **libgpiod 版本差异**：不同平台 libgpiod 版本可能不同，需通过适配层屏蔽差异。
4. **行为差异**：sysfs 实现每次操作都 export/unexport，而 character device 实现每次请求/释放 line，两者在引脚被占用时的行为略有差异，需在文档中说明。

## 10. 实施步骤

1. 在目标硬件上确认 `/dev/gpiochipN` 布局（MIO/EMIO 的 chip 归属与 line 偏移）。
2. 在 [`Makefile.lib`](../spider_cpu/project-spec/meta-user/recipes-apps/plf-hardware-access-ct11/files/Makefile.lib:1) 中新增 `-lgpiod` 依赖。
3. 重写 [`gpio.c`](../spider_cpu/project-spec/meta-user/recipes-apps/plf-hardware-access-ct11/files/gpio.c:1)，实现第 5 节的逻辑。
4. 修复原 `getPinConfig()` 的读写模式 bug（新实现中已通过 `gpiod_line_direction()` 规避）。
5. 更新 [`README`](../spider_cpu/project-spec/meta-user/recipes-apps/plf-hardware-access-ct11/files/README:21) 中关于 GPIO 访问方式的描述。
6. 执行第 8 节的测试计划。
