# 常见 I2C 设备类设计

## 1. 概述

在 [`I2c`](../src/include/libobject/drivers/i2c/I2c.h) 基类之上，为常见 I2C 总线设备创建**具体设备类**。每个设备类封装该芯片的寄存器协议（寄存器地址、数据格式、字节序），复用 `I2c` 的 `read`/`write`/`transfer` 原子事务。

### 1.1 设计目标

- **复用 I2c 基类**：每个设备类继承 `I2c`，复用 `open`/`read`/`write`/`transfer` 原子事务。
- **封装设备协议**：每个类封装该芯片的寄存器地址、数据格式、字节序。
- **从机地址**：每个设备类在构造时指定 `slave_addr`（或通过 `set` 配置）。
- **寄存器地址宽度**：EEPROM 用 8 位，ADC 可能用 16 位，通过 `set_width` 配置。

### 1.2 类层次结构

```
Obj -> I2c -> I2c_EEPROM
           -> I2c_TempSensor
           -> I2c_ADC
           -> I2c_DAC
           -> I2c_GPIO_Expander
```

## 2. 设备类总览

| 设备类 | 典型芯片 | 核心接口 | 寄存器地址宽度 |
|--------|----------|----------|----------------|
| `I2c_EEPROM` | at24c02 | `read_byte`/`write_byte`/`read_bytes`/`write_bytes` | 8 位（at24c02），16 位（at24c32+） |
| `I2c_TempSensor` | tmp105/lm75 | `read_temp` | 8 位 |
| `I2c_ADC` | ads1115 | `read_channel` | 8 位 |
| `I2c_DAC` | mcp4725 | `write_value` | 8 位 |
| `I2c_GPIO_Expander` | pca9535 | `set_pin`/`get_pin`/`set_direction` | 8 位 |

## 3. I2c_EEPROM 类（先实现）

### 3.1 类层次

```
Obj -> I2c -> I2c_EEPROM
```

### 3.2 接口设计

```c
typedef struct I2c_EEPROM_s I2c_EEPROM;

struct I2c_EEPROM_s {
    I2c parent;   /* 继承 I2c 基类 */

    int (*construct)(I2c_EEPROM *, char *);
    int (*deconstruct)(I2c_EEPROM *);
    int (*set)(I2c_EEPROM *, char *attrib, void *value);
    void *(*get)(I2c_EEPROM *, char *attrib);
    char *(*to_json)(I2c_EEPROM *);

    /* EEPROM 接口（复用 I2c 的 read/write） */
    int (*read_byte)(I2c_EEPROM *eeprom, uint16_t addr, uint8_t *data);
    int (*write_byte)(I2c_EEPROM *eeprom, uint16_t addr, uint8_t data);
    int (*read_bytes)(I2c_EEPROM *eeprom, uint16_t addr, uint8_t *buf, size_t len);
    int (*write_bytes)(I2c_EEPROM *eeprom, uint16_t addr, const uint8_t *buf, size_t len);

    /* 属性 */
    int slave_addr;   /* 从机地址 */
    uint32_t size;    /* EEPROM 容量（字节） */
};
```

### 3.3 实现要点

| 接口 | 实现（复用 I2c） |
|------|------------------|
| `read_byte(addr, data)` | `read(i2c, slave_addr, addr, NULL, 0, data, 1)` |
| `write_byte(addr, data)` | `write(i2c, slave_addr, addr, &data, 1)` |
| `read_bytes(addr, buf, len)` | `read(i2c, slave_addr, addr, NULL, 0, buf, len)` |
| `write_bytes(addr, buf, len)` | `write(i2c, slave_addr, addr, buf, len)` |

- `addr` 为 EEPROM 存储地址（寄存器地址），按 `reg_width` 编码（at24c02 用 8 位，at24c32+ 用 16 位）。
- 复用 `I2c` 的原子事务，保证"写地址 + 读数据"不被其他线程打断。

### 3.4 文件规划

| 文件 | 作用 |
|------|------|
| `src/include/libobject/drivers/i2c/I2c_EEPROM.h` | EEPROM 类头文件 |
| `src/drivers/i2c/I2c_EEPROM.c` | EEPROM 类实现 |
| `tests/drivers/test_i2c_eeprom.c` | EEPROM 测试 |

## 4. I2c_TempSensor 类（已实现）

### 4.1 典型芯片：tmp105/lm75

tmp105 寄存器：
- 0x00：温度寄存器（16 位，高字节在前）
- 0x01：配置寄存器
- 0x02：上限温度
- 0x03：下限温度

### 4.2 接口设计

```c
struct I2c_TempSensor_s {
    I2c parent;

    int (*init)(I2c_TempSensor *sensor, int bus_number, int slave_addr);  /* 初始化 */
    int (*read_temp)(I2c_TempSensor *sensor, float *temp);   /* 读温度（摄氏度） */
    int (*set_limit)(I2c_TempSensor *sensor, float high, float low);  /* 设置上下限 */

    /* 可配置属性（通用化，适配不同型号） */
    uint8_t temp_reg;      /* 温度寄存器地址（默认 0x00） */
    float resolution;      /* 分辨率 °C/LSB（默认 0.0625） */
    int data_bits;         /* 有效数据位数（默认 12） */
    bool big_endian;       /* 是否高字节在前（默认 true） */
};
```

### 4.3 实现要点

- `read_temp`：读温度寄存器（`temp_reg`，16 位），按 `data_bits` 有效位数和 `resolution` 分辨率转换温度。
- 型号相关参数（温度寄存器地址、分辨率、数据位数、字节序）作为可配置属性，不硬编码特定型号。
- 复用 `I2c` 的 `read` 原子事务。

### 4.4 测试

温度传感器测试分为两个用例，分别验证默认参数（tmp105）和通用化配置（emc1413）。

#### 4.4.1 tmp105 测试

tmp105 参数（即 `I2c_TempSensor` 的默认配置）：

| 参数 | 值 |
|------|-----|
| 温度寄存器 | 0x00 |
| 寄存器字节数 | 2 |
| 有效数据位数 | 12 |
| 分辨率 | 0.0625°C/LSB |
| 对齐方式 | 左对齐 |

在 QEMU 中挂载 tmp105 温度传感器（地址 0x48，默认 25°C），运行测试：

```sh
mkdir -p /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt
ls /mnt/bin/xtools
export LD_LIBRARY_PATH=/mnt/lib
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_i2c_tempsensor_tmp105
```

预期日志：
```
[INFO]-[tempsensor init success, bus:0, fd:6, slave_addr:0x48]
[INFO]-[tempsensor read_temp ok, raw:0x1900, temp:25.00]
[INFO]-[tempsensor temp verify ok, temp:25.00 C]
```

#### 4.4.2 emc1413 通用性测试

为验证 `I2c_TempSensor` 驱动不硬编码特定型号，在 QEMU 中额外挂载了
**emc1413** 温度传感器（地址 0x4C，设为 30°C）。emc1413 参数与 tmp105 不同：

| 参数 | emc1413 |
|------|---------|
| 温度寄存器 | 0x00 |
| 寄存器字节数 | 1 |
| 有效数据位数 | 8 |
| 分辨率 | 1°C/LSB |
| 对齐方式 | 右对齐 |

> 与 tmp105 的差异：寄存器字节数 2→1、有效位数 12→8、分辨率 0.0625→1°C/LSB、
> 对齐方式 左→右。这些差异通过 `I2c_TempSensor` 的通用化属性配置适配。

测试用例 `test_i2c_tempsensor_emc141x` 通过配置不同的通用化属性
（`temp_reg_len=1`、`resolution=1.0`、`data_bits=8`、`data_align=右对齐`），
复用同一个 `I2c_TempSensor` 驱动读取 emc1413 温度：

```sh
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_i2c_tempsensor_emc141x
```

预期日志：
```
[INFO]-[tempsensor init ok, bus:0, slave_addr:0x4c]
[INFO]-[tempsensor read_temp ok, raw:0x1e, temp:30.00]
[INFO]-[tempsensor temp verify ok, temp:30.00 C]
```

> 结论：`I2c_TempSensor` 通过可配置属性（寄存器字节数、分辨率、有效位数、
> 对齐方式、字节序）即可适配不同型号的温度传感器，无需修改驱动代码。

## 5. I2c_ADC 类（后续实现）

### 5.1 典型芯片：ads1115

ads1115 寄存器：
- 0x00：转换结果（16 位）
- 0x01：配置寄存器（通道选择、增益、采样率）

### 5.2 接口设计

```c
struct I2c_ADC_s {
    I2c parent;

    int (*read_channel)(I2c_ADC *adc, int channel, int16_t *value);  /* 读指定通道 */
    int (*set_gain)(I2c_ADC *adc, int gain);   /* 设置增益 */
};
```

### 5.3 实现要点

- `read_channel`：写配置寄存器选择通道，读转换结果寄存器（16 位）。
- 复用 `I2c` 的 `read`/`write` 原子事务。

## 6. I2c_DAC 类（后续实现）

### 6.1 典型芯片：mcp4725

mcp4725 寄存器：
- 0x40：写 DAC 寄存器（12 位数据）

### 6.2 接口设计

```c
struct I2c_DAC_s {
    I2c parent;

    int (*write_value)(I2c_DAC *dac, uint16_t value);   /* 写 DAC 输出值 */
};
```

### 6.3 实现要点

- `write_value`：写 12 位 DAC 值到寄存器 0x40。
- 复用 `I2c` 的 `write` 原子事务。

## 7. I2c_GPIO_Expander 类（后续实现）

### 7.1 典型芯片：pca9535

pca9535 寄存器：
- 0x00/0x01：输入端口
- 0x02/0x03：输出端口
- 0x06/0x07：配置（方向）

### 7.2 接口设计

```c
struct I2c_GPIO_Expander_s {
    I2c parent;

    int (*set_pin)(I2c_GPIO_Expander *exp, int pin, int level);   /* 设置引脚电平 */
    int (*get_pin)(I2c_GPIO_Expander *exp, int pin, int *level);  /* 读取引脚电平 */
    int (*set_direction)(I2c_GPIO_Expander *exp, int pin, int dir); /* 设置方向 */
};
```

### 7.3 实现要点

- `set_pin`：读-改-写输出寄存器（0x02/0x03）。
- `get_pin`：读输入寄存器（0x00/0x01）。
- `set_direction`：读-改-写配置寄存器（0x06/0x07）。
- 复用 `I2c` 的 `read`/`write` 原子事务。

## 8. 测试规划

每个设备类对应一个测试文件，在 QEMU 中验证：

| 测试文件 | 验证内容 |
|----------|----------|
| `test_i2c_eeprom.c` | at24c02 读写（write + read + 一致性） |
| `test_i2c_tempsensor.c` | 温度读取（tmp105 + emc1413 通用性验证） |
| `test_i2c_adc.c` | ADC 通道读取 |
| `test_i2c_dac.c` | DAC 输出 |
| `test_i2c_gpio.c` | GPIO 引脚读写 |

## 9. 实施顺序

1. **I2c_EEPROM**（先实现，与 QEMU 中已挂载的 at24c02 对应，可直接测试）
2. I2c_TempSensor
3. I2c_ADC / I2c_DAC
4. I2c_GPIO_Expander
