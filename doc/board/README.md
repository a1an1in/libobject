# 板级驱动开发文档（Board Drivers）

板级（`src/board`）相关的用户态驱动开发文档索引。环境搭建与各设备驱动开发按主题拆分成独立文档，
按需查阅。

## 环境搭建

| 文档 | 说明 |
|------|------|
| [`Ubuntu 环境里搭建qemu开发环境.md`](<Ubuntu 环境里搭建qemu开发环境.md>) | QEMU 编译、交叉编译工具链、ARM64 内核 / busybox / initramfs、启动 ARM Linux 系统 |

## 用户态驱动开发

| 文档 | 说明 |
|------|------|
| [`uio_fpga驱动开发.md`](uio_fpga驱动开发.md) | **UIO FPGA 驱动开发**（推荐入口：QEMU `vfpga` 设备 → 设备树 → 内核 UIO → `Uio_Fpga` 类 → 测试的完整流程） |
| [`i2c用户态驱动设计.md`](i2c用户态驱动设计.md) | 通用用户态 I2C 驱动（`I2c` 类） |
| [`spi设计文档.md`](spi设计文档.md) | 用户空间 SPI 驱动（`Spi` 类，基于 spidev，QEMU PL022 + n25q064 测试） |
| [`mtd使用指南.md`](mtd使用指南.md) | MTD Flash 设备使用指南（pflash CFI NOR，`/dev/mtdX` + ioctl/read/write） |
| [`pcie设计文档.md`](pcie设计文档.md) | PCIe 用户态驱动（`Uio_Pcie` 类，基于 uio_pci_generic + mmap BAR） |
| [`vfio设计文档.md`](vfio设计文档.md) | VFIO 用户态驱动（`Vfio` / `Vfio_Pcie` / `Vfio_Pcie_Edu` 类，含 DMA 与中断） |

## 驱动覆盖情况（已实现 / 待实现）

**已实现**（`src/board/hal/` + `src/board/service/`）：

| 驱动 | 实现 | 说明 |
|------|------|------|
| GPIO | [`Gpio.c`](../src/board/hal/gpio/Gpio.c) | gpiochip 字符设备，含边沿事件中断 |
| I2C | [`I2c.c`](../src/board/hal/i2c/I2c.c) | I2C 总线访问 |
| I2C EEPROM | [`I2c_EEPROM.c`](../src/board/hal/i2c/I2c_EEPROM.c) | 24Cxx EEPROM |
| I2C 温度传感器 | [`I2c_TempSensor.c`](../src/board/hal/i2c/I2c_TempSensor.c) | 温度传感器 |
| SPI | [`Spi.c`](../src/board/hal/spi/Spi.c) | spidev 用户态 SPI |
| MTD/Flash | [`Mtd.c`](../src/board/hal/mtd/Mtd.c) | `/dev/mtdX` Flash 访问 |
| UIO 家族 | [`Uio.c`](../src/board/hal/uio/Uio.c) / [`Uio_Pcie.c`](../src/board/hal/uio/Uio_Pcie.c) | 平台/PCIe/FPGA |
| VFIO 家族 | [`Vfio.c`](../src/board/hal/vfio/Vfio.c) / [`Vfio_Pcie.c`](../src/board/hal/vfio/Vfio_Pcie.c) | IOMMU / DMA / 中断 |
| EEPROM 存储服务 | [`Eeprom_Storage.c`](../src/board/service/eeprom_storage/Eeprom_Storage.c) | service 层存储服务 |

**待实现（驱动缺口）**：

| 驱动 | 用户态接入点 | 优先级 |
|------|------------|--------|
| Watchdog | `/dev/watchdog` | 高（产品稳定性刚需） |
| PWM | `/sys/class/pwm` | 高（背光/蜂鸣器/电机） |
| RTC | `/dev/rtc0` | 中 |
| ADC / DAC | IIO `/sys/bus/iio` 或 I2C | 中 |
| Input / evdev | `/dev/input/event*` | 中（按键/触摸） |
| CAN | SocketCAN | 低（用到再补） |
| USB | libusb | 低（用到再补） |

> **UART**：控制台/调试串口由内核 + getty 使用，**无需应用层驱动**；只有驱动外设串口
> （GPS/传感器/Modem）才需要 `Uart` HAL 类，按需再补。

## 架构与设计

| 文档 | 说明 |
|------|------|
| [`嵌入式软件分层架构设计.md`](嵌入式软件分层架构设计.md) | `src/board` 的 HAL 层 / 服务层 / 应用层分层设计 |

## 快速上手

- 环境没搭好，先看 [`Ubuntu 环境里搭建qemu开发环境.md`](<Ubuntu 环境里搭建qemu开发环境.md>)。
- 想动手写一个用户态驱动，推荐从 [`uio_fpga驱动开发.md`](uio_fpga驱动开发.md) 开始——
  「QEMU 造设备（`vfpga` + SPI 70 中断）→ 改设备树 → 改内核启用 UIO → 写用户态驱动
  （`Uio_Fpga` 类）→ 手动 + `test_uio_fpga_*` 测试」全流程的完整示例。
