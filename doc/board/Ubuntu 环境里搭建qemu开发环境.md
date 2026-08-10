Ubuntu 环境里搭建qemu开发环境

## 1. 编译qemu

```
# 1. 克隆源码
git clone https://gitlab.com/qemu-project/qemu
cd qemu

# 2. 创建独立的构建目录（这是推荐的做法，保证源码整洁）[citation:1][citation:4]
mkdir build && cd build
# 3.0 这里有可能需要先安装依赖
sudo apt install -y \
    git \
    ninja-build \
    pkg-config \
    libglib2.0-dev \
    libpixman-1-dev \
    libfdt-dev \
    zlib1g-dev \
    python3-venv \
    python3-pip
# 3.1 运行 configure，这会检查环境并生成 Meson 配置
#    这里我们只编译 ARM 系统模拟部分，可以节省大量时间
../configure --target-list=aarch64-softmmu,aarch64-linux-user --enable-debug

# 4. 使用 Ninja 执行编译
ninja

# 5. 加入环境变量
echo 'export PATH=$PATH:/home/alan/workspace/qemu/build' >> ~/.bashrc
source ~/.bashrc
```

系统模拟器和用户态模拟器是完全不同的东西：

| 模拟器                | 用途                                       | 典型命令                                                     |
| :-------------------- | :----------------------------------------- | :----------------------------------------------------------- |
| `qemu-system-aarch64` | 模拟完整的 ARM 开发板，可以启动 Linux 内核 | qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel Image -initrd rootfs.cpio.gz |
| `qemu-aarch64`        | 在 x86 上直接运行单个 ARM 应用程序         | `qemu-aarch64 ./hello_arm64`                                 |

## 2. 安装交叉编译工具链

```
sudo apt install gcc-aarch64-linux-gnu -y
```

## 3. 编写、编译与验证ARM 程序

### 3.1 编写验证程序

```
// hello.c
#include <stdio.h>
int main() {
    printf("Hello ARM from WSL2!\n");
    return 0;
}
```

### 3.2 编译

```
# 编译成 64位 ARM 程序
aarch64-linux-gnu-gcc test.c -o test_arm64
# 验证文件类型，确认是 ARM 架构
file test_arm64
```

### 3.3 验证用户态模拟器

```
qemu-aarch64 -L /usr/aarch64-linux-gnu ./test_arm64
```

### 3.4 验证系统模拟器

#### 3.4.1 编译linux内核

* 版本选择

​       选用 ‌**linux-4.9.263**‌ 版本，这是一个经典的长期支持版本，稳定且兼容性好，适合在 QEMU 中模拟 ARM 平台。

* 下载链接

  ```
  wget https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.9.263.tar.xz
  ```
  
* 编译步骤

  ```
  tar -xvf linux-4.9.263.tar.xz
  cd linux-4.9.263
  
  # 生成 ARM 平台默认配置
  make ARCH=arm64 defconfig
  
  # 编译内核（使用多线程加速）
  make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc)
  ```
  
编译完成后，内核镜像位于 `arch/arm64/boot/zImage`，在 ARM64 的 virt 平台上，编译内核后‌**不会生成单独的 DTB 文件**。

#### 3.4.2 编译busybox

* 版本选择

  选用 ‌**busybox-1.32.1**‌ 版本，该版本稳定可靠，与 4.9 内核配合良好。

* 下载链接

  ```
  wget https://busybox.net/downloads/busybox-1.32.1.tar.bz2
  ```
  
* 编译步骤

  > **说明**：为了支持动态链接的应用程序（如 libobject 生成的 `xtools`），busybox 需要**动态编译**，并把 aarch64 的动态库（libc、动态链接器等）一起打包进文件系统。

  ```
  tar -xjf busybox-1.32.1.tar.bz2
  cd busybox-1.32.1
  
  # 配置为动态编译（不要勾选 Build static binary）
  make ARCH=arm menuconfig
  
  在配置界面中进行以下设置：
  进入 Settings → Build Options
  取消勾选 [ ] Build static binary (no shared libs)   ← 关键：改为动态编译
  设置 Cross compiler prefix 为 aarch64-linux-gnu-
  进入 Networking Utilities，按空格键取消勾选 tc（避免之前遇到的编译错误）
  保存并退出
  ```

  如果使用命令行直接修改 `.config`（无需 menuconfig）：

  ```
  # 将静态编译改为动态编译
  sed -i 's/^CONFIG_STATIC=y/# CONFIG_STATIC is not set/' .config
  sed -i 's/^CONFIG_STATIC_LIBGCC=y/# CONFIG_STATIC_LIBGCC is not set/' .config
  ```

  ```
  make ARCH=arm CROSS_COMPILE=aarch64-linux-gnu-  -j$(nproc)
  make ARCH=arm CROSS_COMPILE=aarch64-linux-gnu-  install
  ```

  安装完成后会在当前目录生成 `_install` 文件夹。

* 复制动态库到文件系统（动态编译必需）

  动态编译的 busybox 依赖 `libc.so.6`、`ld-linux-aarch64.so.1`、`libm.so.6`、`libresolv.so.2`，必须将这些库复制到 `_install/lib`，否则系统无法启动：

  ```
  cd _install
  mkdir -p lib
  cp /usr/aarch64-linux-gnu/lib/libc.so.6 lib/
  cp /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1 lib/
  cp /usr/aarch64-linux-gnu/lib/libm.so.6 lib/
  cp /usr/aarch64-linux-gnu/lib/libresolv.so.2 lib/
  ```

  验证 busybox 是动态链接：

  ```
  file _install/bin/busybox
  # 期望输出: ELF 64-bit LSB pie executable, ARM aarch64, dynamically linked
  ```

* 补充必要目录和文件

  ```
  cd _install
  mkdir -p etc dev proc sys tmp etc/init.d
  ```
  
  创建 `etc/fstab`：
  
  ```
  cat > etc/fstab << EOF
  proc    /proc    proc    defaults    0    0
  tmpfs   /tmp     tmpfs   defaults    0    0
  sysfs   /sys     sysfs   defaults    0    0
  EOF
  ```
  
  创建 `etc/init.d/rcS`（初始化脚本）：

  > **注意**：
  > 1. 必须设置 `HOME` 环境变量。libobject 的 debugger 依赖 `HOME` 创建 `~/.xtools/dbg.ini`，若 `HOME` 未设置，`xtools` 启动时会崩溃（`getenv("HOME")` 返回 NULL）。
  > 2. 必须配置回环接口 `lo`。libobject 的网络组件（`producer`、`event_base`）启动时会绑定 UDP socket 到 `127.0.0.1`，若 `lo` 未配置，会报 `bind error for 127.0.0.1` / `connect error for 127.0.0.1`。

  ```
  cat > etc/init.d/rcS << 'EOF'
  #!/bin/sh
  # 设置 HOME 环境变量（libobject 的 debugger 依赖 HOME 创建 ~/.xtools/dbg.ini）
  export HOME=/root
  mkdir -p /root
  mount -a
  mkdir -p /dev/pts
  mount -t devpts devpts /dev/pts
  echo /sbin/mdev > /proc/sys/kernel/hotplug
  mdev -s
  # 配置回环接口（libobject 网络组件依赖 127.0.0.1）
  ifconfig lo 127.0.0.1 up
  EOF
  
  chmod +x etc/init.d/rcS
  ```
  
  创建 `etc/inittab`：
  
  ```
  cat > etc/inittab << EOF
  ::sysinit:/etc/init.d/rcS
  ::respawn:-/bin/sh
  ::askfirst:-/bin/sh
  ::ctrlaltdel:/bin/umount -a -r
  EOF
  ```
  
  创建基本设备节点：
  
  ```
  cd dev
  sudo mknod console c 5 1
  sudo mknod null c 1 3
  cd ..
  ```
  
  打包文件系统
  
  ```
  find . | cpio -o -H newc | gzip > ../initramfs.cpio.gz
  zcat ../initramfs.cpio.gz | cpio -t
  ```

#### 3.4.3 用 QEMU 启动系统

```
qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a57 \
  -m 2G \
  -kernel ~/workspace/linux-4.9.263/arch/arm64/boot/Image \
  -initrd ~/workspace/busybox-1.33.1/initramfs.cpio.gz \
  -nographic \
  -append "console=ttyAMA0 rdinit=/linuxrc"
```

启动后按回车，就能看到 shell 提示符，一个完整的 ARM Linux 系统就在 QEMU 中运行起来了。

## 4. 开发UIO 应用层驱动

开发 UIO 应用层驱动分为四步：**创建虚拟 fpga 设备**、**修改 Linux 内核**、**实现 libobject FPGA 驱动**、**测试**。

### 4.1 创建虚拟 fpga 设备

创建虚拟 fpga 设备需要两步：**修改 QEMU 源码添加 `vfpga` 模拟设备**（让 QEMU 能模拟一个会触发中断的 FPGA），**修改设备树**（让 guest 内核识别该设备）。

#### 4.1.1 概述：修改 QEMU 添加 vfpga 模拟设备

QEMU `virt` 机器默认没有 FPGA 外设模型，`0x0b000000` 地址上没有任何设备，写寄存器不会触发中断。因此需要在 QEMU 源码中新增一个 `vfpga`（Virtual FPGA）模拟设备，guest 写其寄存器即可触发 SPI 70 中断。

修改涉及以下文件：

| 文件 | 修改内容 |
|------|---------|
| `include/hw/misc/vfpga.h` | 新增：vfpga 设备模型头文件（定义寄存器偏移、状态结构） |
| `hw/misc/vfpga.c` | 新增：vfpga 设备模型实现（MMIO 读写、中断控制） |
| `include/hw/arm/virt.h` | 添加 `VIRT_FPGA` 枚举项 |
| `hw/arm/virt.c` | 分配 MMIO 地址 `0x0b000000`、连接 GIC SPI 70、生成设备树节点、调用 `create_vfpga()` |
| `hw/misc/Kconfig` | 添加 `VFPGA` 配置 |
| `hw/misc/meson.build` | 添加编译条目 |
| `hw/arm/Kconfig` | `ARM_VIRT` 添加 `select VFPGA` |

`vfpga` 设备模型的核心逻辑（`hw/misc/vfpga.c`）：

```c
/* 中断控制寄存器 0xFF0：写 bit0=1 触发中断，bit0=0 清除 */
case VFPGA_REG_IRQ_CTRL:   // 0xFF0
    if (value & 1) {
        s->irq_pending = 1;
        qemu_set_irq(s->irq, 1);   // 拉高中断线 → GIC 收到 SPI 70
    } else {
        s->irq_pending = 0;
        qemu_set_irq(s->irq, 0);   // 拉低中断线
    }
    break;
```

在 `hw/arm/virt.c` 中，将设备的中断输出连接到 GIC 的 SPI 70：

```c
static void create_vfpga(const VirtMachineState *vms)
{
    ...
    s = SYS_BUS_DEVICE(qdev_new(TYPE_VFPGA));
    sysbus_realize_and_unref(s, &error_fatal);
    sysbus_mmio_map(s, 0, base);                       // 映射到 0x0b000000
    sysbus_connect_irq(s, 0, qdev_get_gpio_in(vms->gic, irq));  // 连接 GIC SPI 70
    ...
}
```

修改完成后重新编译 QEMU：

```bash
cd ~/workspace/qemu/build
ninja qemu-system-aarch64
```

> **说明**：`vfpga` 设备寄存器布局（相对 `0x0b000000`）：`0x00`-`0xFEC` 为数据寄存器（支持 32/64 位读写），`0xFF0` 为中断控制寄存器（写 bit0=1 触发中断），`0xFF4` 为中断状态寄存器（只读）。

#### 4.1.2 导出默认设备树

```
qemu-system-aarch64 \
  -M virt,dumpdtb=virt_default.dtb \
  -cpu cortex-a57 \
  -m 2G \
  -kernel ~/workspace/linux-4.9.263/arch/arm64/boot/Image \
  -nographic
```

#### 4.1.3 反编译为可编辑格式

```
dtc -I dtb -O dts virt_default.dtb -o virt_custom.dts
```

#### 4.1.4 修改设备树节点

````
对于你的虚拟 FPGA 设备，建议使用70，这个编号明确未被占用且远离已分配区域：
fpga@b000000 {
    compatible = "generic-uio";
    reg = <0x0 0xb000000 0x0 0x1000>;
    interrupts = <0 70 4>;
    interrupt-parent = <&intc>;
};
````

> **注意**：`0x0b000000` 是 QEMU `virt` 机器中为 FPGA 模拟设备预留的 MMIO 地址（见 `hw/arm/virt.c` 的 `VIRT_FPGA`）。该地址对应 QEMU 中新增的 `vfpga` 模拟设备（`hw/misc/vfpga.c`），guest 写其 `0xFF0` 寄存器（bit0=1）即可触发 SPI 70 中断。若使用 QEMU 自动生成的设备树，该节点会自动生成，无需手动添加。

#### 4.1.5 编译回 DTB

```
dtc -I dts -O dtb virt_custom.dts -o virt_custom.dtb
```

### 4.2 修改 Linux 内核（启用 UIO 驱动）

许多嵌入式或通用内核的默认配置为了精简体积，会关闭非核心驱动。请按照以下步骤强制启用 UIO 支持：

#### 4.2.1 直接修改 `.config` 文件

打开内核根目录下的 `.config` 文件，找到以下行（如果存在）：

```
# CONFIG_UIO is not set
```

将其修改为：

```
textCONFIG_UIO=y
CONFIG_UIO_PDRV_GENIRQ=y
```

*注意：如果文件中完全没有 `CONFIG_UIO` 相关行，直接在文件末尾添加这两行即可。*

#### 4.2.2 更新配置依赖

保存文件后，执行以下命令以解析依赖关系并更新配置：

```
make ARCH=arm64 olddefconfig
```

这一步会根据你手动添加的 `CONFIG_UIO=y` 自动检查并启用必要的依赖项。

#### 4.2.3 验证配置

再次检查 `.config` 文件，确认配置已生效：

```
grep CONFIG_UIO .config
```

输出应显示：

```
textCONFIG_UIO=y
CONFIG_UIO_PDRV_GENIRQ=y
```

#### 4.2.4 重新编译内核

配置生效后，必须重新编译内核才能将 UIO 驱动包含进去：

```
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc)
```

#### 4.2.5 使用新的 DTB 和内核运行

```
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

#### 4.2.6 验证 UIO 设备

```
dmesg | grep -i uio
ls /dev/uio*
```

### 4.3 实现 libobject FPGA 驱动

#### 4.3.1 编译 libobject（aarch64）

在 libobject 仓库根目录执行：

```bash
./devops.sh build --platform=linux --arch=aarch64
```

编译产物位于 `sysroot/linux/aarch64/bin/xtools`，动态库位于 `sysroot/linux/aarch64/lib/`。

#### 4.3.2 部署代码到 QEMU（推荐：9p 共享目录）

由于代码会经常修改，推荐使用 **9p virtfs 共享目录**，宿主机与 guest 共享一个目录，改代码后只需重新编译，guest 里直接看到新文件，**无需重启 QEMU**。

内核已确认支持 9p（`CONFIG_NET_9P=y`、`CONFIG_NET_9P_VIRTIO=y`、`CONFIG_9P_FS=y`）。
启动 QEMU 时追加 `-virtfs` 参数，共享 libobject 的 `sysroot/linux/aarch64` 目录：

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

### 4.4 测试

测试分为两部分：**手动测试**（用 shell 命令直接验证）和 **case 测试**（运行 libobject 的自动化测试用例）。

#### 4.4.1 手动测试

##### 验证 UIO 设备

```sh
# 查看 UIO 设备
ls -l /dev/uio*
cat /sys/class/uio/uio0/name          # 期望: fpga
cat /sys/class/uio/uio0/maps/map0/addr # 期望: 0x0b000000
cat /sys/class/uio/uio0/maps/map0/size # 期望: 0x1000
```

##### 验证寄存器读写

```sh
# 用 devmem 读写寄存器（若 busybox 有 devmem）
devmem 0x0b000000 32 0x12345678
devmem 0x0b000000 32                  # 期望: 0x12345678
```

##### 验证中断

手动验证中断只需验证**第一次中断**（计数从 0 变 1），简单可靠：

```sh
# 1. 查看当前中断计数（初始应为 0）
cat /sys/class/uio/uio0/event

# 2. 触发中断（写 vfpga 中断控制寄存器 0xFF0 的 bit0=1）
devmem 0x0b000ff0 32 1

# 3. 再次查看中断计数，应 +1
cat /sys/class/uio/uio0/event         # 期望: 1

# 4. 清除中断（高电平触发需复位，写 0xFF0 = 0）
devmem 0x0b000ff0 32 0
```

> **说明**：
> - `/sys/class/uio/uio0/event` 只读当前中断计数，**不阻塞**，触发中断后计数 +1。
> - 中断为高电平触发（`interrupts = <0 70 4>`），触发后必须写 `0x0b000ff0` 为 0 清除，否则中断持续触发。
> - **多次中断递增验证**（计数 +2、+3 等）在 QEMU 中较复杂：`uio_pdrv_genirq` 在每次中断后自动屏蔽中断，需读 `/dev/uio0` 重新使能，且读操作是阻塞的，时序难以控制。**推荐用 case 测试 `test_uio_fpga_irq` 验证多次中断**（自动触发 + 等待 + 验证计数，已确认可靠）。

#### 4.4.2 case 测试

##### 运行 UIO 驱动测试（test_uio）

```sh
mkdir -p /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt
export LD_LIBRARY_PATH=/mnt/lib
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_uio
```

测试通过时日志显示 `test suc, func_name = test_uio`，且 `read register[0x0] = 0xdeadbeef`。

##### 中断测试（test_uio_fpga_irq，自动触发）

`test_uio_fpga_irq` 测试已实现**自动触发中断**：先使能中断，再写 `0xFF0` 触发 SPI 70 中断，然后 `wait_irq` 等待并验证中断计数。运行测试即可，无需手动注入：

```sh
mkdir -p /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt
ls /mnt/bin/xtools 
export LD_LIBRARY_PATH=/mnt/lib
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_uio_fpga_irq
```

预期日志：

```
[INFO]-[enable_irq ok]
[INFO]-[trigger irq ok (write 0xFF0 = 1)]
[INFO]-[wait_irq ok, irq_count:1]
[INFO]-[disable_irq ok]
[WIP]-[command suc, func_name = test_uio_fpga_irq, ...]
```
