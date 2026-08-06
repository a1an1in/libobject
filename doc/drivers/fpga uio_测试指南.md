# FPGA UIO 驱动 QEMU 测试指南

## 1. 环境说明

QEMU 模拟环境（aarch64）：

```bash
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

设备树节点（`virt_custom.dtb` 中）：

```dts
fpga@50000000 {
    compatible = "generic-uio";
    reg = <0x0 0x50000000 0x0 0x1000>;
    interrupts = <0 70 4>;
    interrupt-parent = <&intc>;
};
```

> **注意**：在 QEMU `virt` 机器中，`0x50000000` 是普通 RAM（非真实 FPGA 外设）。因此 UIO 驱动对该区域的读写实际是对 RAM 的读写，这**不影响验证 UIO 驱动机制本身**（设备创建、mmap、寄存器读写、中断框架），只是没有真实硬件行为。

---

## 2. 如何访问 QEMU 环境

### 2.1 启动 QEMU

在宿主机终端执行上面的 QEMU 命令。由于使用了 `-nographic`，QEMU 的串口（`ttyAMA0`）直接映射到当前终端，启动后即可看到内核日志和 busybox 的 shell 提示符。

### 2.2 进入系统

系统启动完成后会进入 busybox shell（`#` 提示符，root 用户）。此时即可执行命令。

### 2.3 退出 QEMU

在串口控制台按 `Ctrl+A` 然后按 `X` 退出 QEMU。

---

## 3. 如何部署代码（推荐：9p 共享目录，方便调试）

由于代码会经常修改，**推荐使用 9p virtfs 共享目录**，宿主机与 guest 共享一个目录，改代码后只需重新编译，guest 里直接看到新文件，**无需重启 QEMU**。

### 3.1 确认内核支持 9p

内核已确认支持（`CONFIG_NET_9P=y`、`CONFIG_NET_9P_VIRTIO=y`、`CONFIG_9P_FS=y`）。

### 3.2 启动 QEMU 时挂载共享目录

在 QEMU 命令中追加 `-virtfs` 参数，共享 libobject 的 `sysroot/linux/aarch64` 目录：

```bash
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

### 3.3 在 guest 中挂载共享目录

进入 guest shell 后：

```sh
mkdir -p /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt
ls /mnt/bin/xtools   # 应能看到编译产物
```

### 3.4 运行 xtools

`xtools` 是动态链接的，依赖 `libobject-core.so`、`libc.so.6`、`ld-linux-aarch64.so.1`。其中 `libc.so.6` 和 `ld-linux-aarch64.so.1` 已在文件系统中（见环境搭建文档 3.4.2 节），`libobject-core.so` 在共享目录 `/mnt/lib` 中。

```sh
export LD_LIBRARY_PATH=/mnt/lib
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery -f test_uio_fpga
```

> **说明**：aarch64 平台已排除 stub 模块，`xtools` 不再依赖 `libobject-stub.so`。

---

## 4. 测试前准备（确认 UIO 设备）

进入 QEMU 系统后，先确认 UIO 驱动已加载、设备节点已生成：

```sh
# 查看 UIO 设备
ls -l /dev/uio*

# 查看 UIO 设备名称（应为 "fpga"）
cat /sys/class/uio/uio0/name

# 查看映射信息
cat /sys/class/uio/uio0/maps/map0/addr
cat /sys/class/uio/uio0/maps/map0/size

# 查看中断事件计数
cat /sys/class/uio/uio0/event
```

预期输出：
- `/dev/uio0` 存在
- `name` 为 `fpga`
- `map0/addr` 为 `0x50000000`
- `map0/size` 为 `0x1000`（4096 字节）

---

## 5. 手动测试（无需编译 libobject）

### 5.1 用 devmem 直接读写寄存器

如果 busybox 有 `devmem`：

```sh
# 写 0xDEADBEEF 到 0x50000000
devmem 0x50000000 32 0xDEADBEEF

# 读回
devmem 0x50000000 32
```

### 5.2 用 hexdump 读取映射区域

```sh
# 读取 /dev/uio0 前 16 字节（即 0x50000000 处）
hexdump -C -n 16 /dev/uio0
```

### 5.3 测试中断（可选）

UIO 中断通过读 `/dev/uio0` 触发（阻塞等待中断事件）：

```sh
# 阻塞等待中断（QEMU 无真实中断源时会一直阻塞，可用 timeout 限制）
timeout 3 cat /dev/uio0
```

---

## 6. 使用 libobject 的 Uio 驱动测试

### 6.1 编译 libobject（aarch64）

```bash
./devops.sh build --platform=linux --arch=aarch64
```

编译产物位于 `sysroot/linux/aarch64/bin/xtools`。

### 6.2 运行 UIO 测试

在 QEMU 系统内（通过 9p 共享目录）运行：

```sh
export LD_LIBRARY_PATH=/mnt/lib
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery -f test_uio_fpga

# 或运行所有测试
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery -f all
```

`test_uio_fpga` 测试流程：
1. 创建 `Uio` 对象
2. `uio->open(uio, "fpga")` — 通过 `/sys/class/uio/uio0/name` 匹配 "fpga"，打开 `/dev/uio0`
3. `uio->mmap(uio)` — mmap 映射 0x50000000 区域
4. `uio->write_register(uio, 0x0, 0xDEADBEEF)` — 写寄存器
5. `uio->read_register(uio, 0x0, &val)` — 读寄存器
6. `uio->write_registers` / `uio->read_registers` — 批量读写
7. 越界访问保护测试
8. `uio->close(uio)`
---

## 7. 测试结果判断

### 7.1 成功标志

`test_uio_fpga` 测试通过时，日志会显示：

```
test suc, func_name = test_uio_fpga, ...
```

并且 `read register[0x0] = 0xdeadbeef`（写回读一致）。

### 7.2 常见问题排查

| 现象 | 可能原因 | 解决方法 |
|------|---------|---------|
| `open` 失败，找不到设备 | 设备树节点未生效 / 内核参数缺失 | 确认 `uio_pdrv_genirq.of_id=generic-uio` 启动参数；确认 dtb 中 `fpga@50000000` 节点存在 |
| `open` 失败，`/dev/uio0` 不存在 | UIO 驱动未加载 | `modprobe uio_pdrv_genirq` 或确认内核编译了 `CONFIG_UIO_PDRV_GENIRQ` |
| `mmap` 失败 | 权限不足 / 设备未打开 | 以 root 运行；确认 `open` 成功 |
| 读写返回越界错误 | offset 超过 0x1000 | 检查传入的 offset 是否在映射范围内 |
| 中断等待超时 | QEMU 无真实中断源 | 中断测试在 QEMU 中无法触发真实中断，仅验证框架 |
| 运行 xtools 报找不到动态库 | 未设置 `LD_LIBRARY_PATH` | 执行 `export LD_LIBRARY_PATH=/mnt/lib` |

---

## 8. 中断测试说明

在 QEMU `virt` 机器中，`fpga@50000000` 节点声明了中断 `interrupts = <0 70 4>`（SPI 70，高电平触发），但 QEMU 没有对应的中断产生设备模型，因此**无法在 QEMU 中触发真实 FPGA 中断**。

`Uio` 驱动已实现中断接口：
- `uio->enable_irq(uio)` — 向 `/dev/uio0` 写 1 使能中断
- `uio->wait_irq(uio, timeout_ms)` — 阻塞等待中断（poll + read）
- `uio->disable_irq(uio)` — 向 `/dev/uio0` 写 0 禁用中断

在真实硬件上，FPGA 产生中断后，`wait_irq` 会返回中断计数。在 QEMU 中可用 `timeout` 验证 `wait_irq` 的超时路径。

---

## 9. 完整测试流程示例

```sh
# 1. 宿主机：编译 aarch64 版本
./devops.sh build --platform=linux --arch=aarch64

# 2. 启动 QEMU（带 -virtfs 共享目录，见第 3.2 节）

# 3. 进入 guest 后，挂载共享目录
mkdir -p /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt

# 4. 确认 UIO 设备
ls -l /dev/uio0
cat /sys/class/uio/uio0/name          # 期望: fpga
cat /sys/class/uio/uio0/maps/map0/size # 期望: 0x1000

# 5. 手动验证寄存器读写（devmem）
devmem 0x50000000 32 0x12345678
devmem 0x50000000 32                  # 期望: 0x12345678

# 6. 运行 libobject UIO 测试
export LD_LIBRARY_PATH=/mnt/lib
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery -f test_uio_fpga
```
