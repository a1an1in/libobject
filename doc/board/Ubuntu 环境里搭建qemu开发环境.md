# Ubuntu 环境里搭建 QEMU 开发环境

> 本文介绍如何在 Ubuntu（WSL2）里从零搭建一套 **QEMU + ARM64 Linux** 开发环境，
> 用于开发板级用户态驱动。搭建完成、能启动系统后，各设备的用户态驱动开发文档见
> [`doc/board/README.md`](README.md) 的索引。

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

​       选用 **linux-4.9.263** 版本，这是一个经典的长期支持版本，稳定且兼容性好，适合在 QEMU 中模拟 ARM 平台。

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
  
编译完成后，内核镜像位于 `arch/arm64/boot/zImage`，在 ARM64 的 virt 平台上，编译内核后**不会生成单独的 DTB 文件**。

#### 3.4.2 编译busybox

* 版本选择

  选用 **busybox-1.32.1** 版本，该版本稳定可靠，与 4.9 内核配合良好。

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

## 4. 下一步：开发用户态驱动

环境就绪后，各设备的用户态驱动开发文档见 [`doc/board/README.md`](README.md) 的索引
（UIO FPGA / I2C / SPI / PCIe / VFIO / MTD），其中 [`uio_fpga驱动开发.md`](uio_fpga驱动开发.md)
是「QEMU 造设备 → 设备树 → 内核 UIO → 用户态驱动 → 测试」全流程的完整示例。
