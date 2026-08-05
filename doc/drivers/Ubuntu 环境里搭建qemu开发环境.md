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

## 4 虚拟fpga设备

### 4.1 导出默认设备树

```
qemu-system-aarch64 \
  -M virt,dumpdtb=virt_default.dtb \
  -cpu cortex-a57 \
  -m 2G \
  -kernel ~/workspace/linux-4.9.263/arch/arm64/boot/Image \
  -nographic
```

### 4.2 反编译为可编辑格式

```
dtc -I dtb -O dts virt_default.dtb -o virt_custom.dts
```

### 4.3 修改设备树节点

````
对于你的虚拟 FPGA 设备，建议使用70，这个编号明确未被占用且远离已分配区域：
fpga@50000000 {
    compatible = "generic-uio";
    reg = <0x0 0x50000000 0x0 0x1000>;
    interrupts = <0 70 4>;
    interrupt-parent = <&intc>;
};
````

### 4.4 编译回 DTB

```
dtc -I dts -O dtb virt_custom.dts -o virt_custom.dtb
```

### 4.5 重新编译Linux内核

许多嵌入式或通用内核的默认配置为了精简体积，会关闭非核心驱动.

解决方法，请按照以下步骤强制启用 UIO 支持：

* 直接修改 `.config` 文件

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

* 更新配置依赖

  保存文件后，执行以下命令以解析依赖关系并更新配置：

  ```
  make ARCH=arm64 olddefconfig
  ```

  这一步会根据你手动添加的 `CONFIG_UIO=y` 自动检查并启用必要的依赖项。

*  验证配置

  再次检查 `.config` 文件，确认配置已生效：

  ```
  grep CONFIG_UIO .config
  ```

  输出应显示：

  ```
  textCONFIG_UIO=y
  CONFIG_UIO_PDRV_GENIRQ=y
  ```

* 重新编译内核

  配置生效后，必须重新编译内核才能将 UIO 驱动包含进去：

  ```
  make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc)
  ```

### 4.6 使用新的DTB和内核运行

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

### 4.7 验证

```
dmesg | grep -i uio
ls /dev/uio*
```

## 5 开发UIO 应用层驱动

### 5.1 编译 libobject（aarch64）

在 libobject 仓库根目录执行：

```bash
./devops.sh build --platform=linux --arch=aarch64
```

编译产物位于 `sysroot/linux/aarch64/bin/xtools`，动态库位于 `sysroot/linux/aarch64/lib/`。

### 5.2 部署代码到 QEMU（推荐：9p 共享目录）

由于代码会经常修改，推荐使用 **9p virtfs 共享目录**，宿主机与 guest 共享一个目录，改代码后只需重新编译，guest 里直接看到新文件，**无需重启 QEMU**。

内核已确认支持 9p（`CONFIG_NET_9P=y`、`CONFIG_NET_9P_VIRTIO=y`、`CONFIG_9P_FS=y`）。

**步骤 1**：启动 QEMU 时追加 `-virtfs` 参数，共享 libobject 的 `sysroot/linux/aarch64` 目录：

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

**步骤 2**：进入 guest 后挂载共享目录：

```sh
mkdir -p /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt
ls /mnt/bin/xtools   # 应能看到编译产物
```

**步骤 3**：运行 xtools（动态库已在文件系统中，直接运行即可）：

```sh
export LD_LIBRARY_PATH=/mnt/lib
/mnt/bin/xtools mockery -f test_uio_fpga
```

> **说明**：
> - aarch64 平台已排除 stub 模块（见 5.6 节），`xtools` 只依赖 `libobject-core.so`、`libc.so.6`、`ld-linux-aarch64.so.1`。
> - `libc.so.6` 和 `ld-linux-aarch64.so.1` 已在文件系统中（见 3.4.2 节），`libobject-core.so` 在共享目录 `/mnt/lib` 中，因此需设置 `LD_LIBRARY_PATH=/mnt/lib`。

### 5.3 手动验证 UIO 设备

```sh
# 查看 UIO 设备
ls -l /dev/uio*
cat /sys/class/uio/uio0/name          # 期望: fpga
cat /sys/class/uio/uio0/maps/map0/addr # 期望: 0x50000000
cat /sys/class/uio/uio0/maps/map0/size # 期望: 0x1000

# 用 devmem 读写寄存器（若 busybox 有 devmem）
devmem 0x50000000 32 0x12345678
devmem 0x50000000 32                  # 期望: 0x12345678
```

### 5.4 运行 UIO 驱动测试

`test_uio_fpga` 测试流程：
1. 创建 `Uio` 对象
2. `uio->open(uio, "fpga")` — 通过 `/sys/class/uio/uio0/name` 匹配 "fpga"，打开 `/dev/uio0`
3. `uio->mmap(uio)` — mmap 映射 0x50000000 区域
4. `uio->write_register` / `uio->read_register` — 单寄存器读写
5. `uio->write_registers` / `uio->read_registers` — 批量读写
6. 越界访问保护测试
7. `uio->close(uio)`

```sh
/mnt/bin/xtools mockery -f test_uio_fpga
```

测试通过时日志显示 `test suc, func_name = test_uio_fpga`，且 `read register[0x0] = 0xdeadbeef`。

### 5.5 中断测试说明

在 QEMU `virt` 机器中，`fpga@50000000` 节点声明了中断 `interrupts = <0 70 4>`，但 QEMU 没有对应的中断产生设备模型，因此**无法在 QEMU 中触发真实 FPGA 中断**。`Uio` 驱动已实现中断接口（`enable_irq` / `wait_irq` / `disable_irq`），在真实硬件上可用，QEMU 中仅能验证 `wait_irq` 的超时路径。

### 5.6 aarch64 平台排除 stub 模块

`stub` 模块依赖动态加载特性（`libobject-stub.so` 需实时加载），在 aarch64 嵌入式平台上暂不支持。构建框架已做如下处理：

- **`mk/linux.cmake`**：aarch64 平台不编译 `src/stub` 模块，且 `xtools` 不链接 `object-stub`（`STUB_LIB` 为空）。
- **`tests/CMakeLists.txt`**：aarch64 平台排除 `tests/stub` 测试文件。
- **`src/argument/Application.c`**、**`src/scripts/fshell/FShell.c`**、**`tests/node/test_node.c`**：用 `#ifndef __aarch64__` 条件编译排除 stub 相关调用（`__aarch64__` 是 GCC 在 aarch64 上自动定义的内置宏，无需手动传参）。

因此 aarch64 平台的 `xtools` 只依赖 `libobject-core.so`、`libc.so.6`、`ld-linux-aarch64.so.1`，不再依赖 `libobject-stub.so`。

> **注意**：32 位 ARM（`--arch=arm`）和 x86_64 平台仍正常编译并链接 stub 模块，不受影响。

