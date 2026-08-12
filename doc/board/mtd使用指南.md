# QEMU virt 机器中模拟 MTD 设备（pflash CFI NOR）使用指南

## 1. 背景

MTD（Memory Technology Device）用于访问 Flash 存储设备（NAND/NOR/QSPI Flash）。
在 QEMU `virt` 机器中，我们使用 **pflash（CFI NOR Flash）** 设备模型来模拟 MTD 设备，
guest 内核启用 MTD 相关驱动后，会出现 `/dev/mtd0`、`/dev/mtdblock0` 等设备节点，
应用层即可通过 `open + ioctl(MEMGETINFO/MEMERASE) + read/write` 访问。

本指南基于 [`Ubuntu 环境里搭建qemu开发环境.md`](<Ubuntu 环境里搭建qemu开发环境.md>) 搭建的
QEMU + Linux 4.9 + busybox 环境，说明如何在 `virt` 机器上挂载 pflash 并运行 libobject 的 MTD 测试。

## 2. 为什么用 pflash 而不是 m25p80

QEMU `virt` 机器的 Flash/MTD 情况：

| 设备模型 | virt 默认是否有 | 说明 |
|---------|---------------|------|
| `pflash_cfi01`（CFI NOR） | ✅ 默认有 | virt 默认创建两个 pflash（`pflash0`/`pflash1`），用于 UEFI 固件，通过 `-drive if=pflash` 挂载 |
| `m25p80`（SPI NOR） | ❌ 默认没有 | 需要 SPI 控制器，而 virt 默认没有 SPI 控制器，需额外添加，较复杂 |
| `nand`（NAND） | ❌ 默认没有 | 需额外配置 |

**结论**：`virt` 机器默认没有 `/dev/mtd*` 设备，但**默认有 pflash（CFI NOR）**，
这是 virt 上最现成、最可靠的 MTD 模拟方式，无需修改 QEMU 源码。

## 3. 在 QEMU virt 中挂载 pflash

### 3.1 创建 Flash 镜像文件

```bash
# 创建一个 64MB 的 CFI NOR Flash 镜像（全 0xFF，模拟出厂状态）
# 注意：virt 机器单个 pflash 槽位（virt.flash0）大小为 64MB，
# 镜像大小必须匹配，否则 QEMU 会报错：
#   "cfi.pflash01 device '/machine/virt.flash0' requires 67108864 bytes"
# 镜像统一放在 qemu_virt_machine 目录（与 virt_custom.dtb 同目录）。
cd ~/workspace/qemu_virt_machine
dd if=/dev/zero of=flash.img bs=1M count=64
tr '\0' '\377' < flash.img > flash_ff.img && mv flash_ff.img flash.img
```

### 3.2 启动 QEMU 并挂载 pflash

> **⚠️ 重要**：启动前必须先执行 3.1 创建 `flash.img` 镜像文件，
> 否则 QEMU 会报 `Could not open 'flash.img': No such file or directory`。
> 镜像统一放在 `~/workspace/qemu_virt_machine/` 目录。

`virt` 机器默认有两个 pflash 槽位（`pflash0`/`pflash1`）：
- **`pflash0` 是固件槽位**（映射到地址 `0x0`），若挂载镜像，QEMU 会尝试从它加载固件，
  空 pflash（全 0xFF）会导致启动卡住。
- **`pflash1` 是数据槽位**（映射到地址 `0x4000000`），可安全挂载 MTD 镜像。

因此**必须用 `-drive if=pflash,index=1` 挂到 `pflash1`**，避免占用固件槽位 `pflash0`，
这样 QEMU 在 `-kernel` 模式下直接加载内核，不会尝试从 pflash 加载固件：

```bash
qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a57 \
  -m 2G \
  -kernel ~/workspace/linux-4.9.263/arch/arm64/boot/Image \
  -dtb ~/workspace/qemu_virt_machine/virt_custom.dtb \
  -initrd ~/workspace/busybox-1.33.1/initramfs.cpio.gz \
  -nographic \
  -drive file=/home/alan/workspace/qemu_virt_machine/flash.img,if=pflash,index=1,format=raw \
  -append "console=ttyAMA0 rdinit=/linuxrc"
```

> **说明**：
> - `-dtb` 指定修复后的设备树（`flash@0` 只覆盖 pflash1，`bank-width=0x02`）。
> - `-drive if=pflash,index=1` 把 flash 镜像挂载到 `pflash1`（数据槽位）。
> - **`console=ttyAMA0` 必须全小写**，大写会导致内核找不到 console 而卡住。

### 3.3 验证 MTD 设备

进入 guest shell 后：

```sh
# 查看 MTD 设备
cat /proc/mtd
# 实际输出：
# dev:    size   erasesize  name
# mtd0: 02000000 00020000  "4000000.flash"

# 查看 /dev/mtd*
ls -l /dev/mtd*
# 实际输出：
# crw-rw----  1 0  0  90,  0  /dev/mtd0
# crw-rw----  1 0  0  90,  1  /dev/mtd0ro
# brw-rw----  1 0  0  31,  0  /dev/mtdblock0
```

> **说明**：`size=0x2000000`（32MB）和 `erasesize=0x20000`（128KB）是
> QEMU pflash_cfi01 在 x16 宽度下 CFI 探测的实际结果。

## 4. 修改 Linux 内核（启用 MTD 驱动）

### 4.1 启用 MTD 相关配置

编辑内核根目录下的 `.config`，确保以下配置开启：

```
CONFIG_MTD=y
CONFIG_MTD_BLOCK=y
CONFIG_MTD_CFI=y
CONFIG_MTD_CFI_AMDSTD=y
CONFIG_MTD_PHYSMAP=y
CONFIG_MTD_PHYSMAP_OF=y
```

> **说明**：
> - `CONFIG_MTD_PHYSMAP_OF` 让内核从设备树解析 `flash@0` 节点并注册为 MTD 设备。
> - `CONFIG_MTD_CFI` + `CONFIG_MTD_CFI_AMDSTD` 提供 CFI NOR Flash 的驱动支持。
> - **`CONFIG_MTD_CHAR` 无需单独配置**：在 Linux 4.9 中，`mtdchar`（`/dev/mtd*` 字符设备）
>   是 MTD 核心的一部分（`drivers/mtd/Makefile` 中 `mtd-y := ... mtdchar.o`），
>   只要 `CONFIG_MTD=y` 就会自动编译。

### 4.2 修改设备树（关键）

`virt` 机器默认的 `flash@0` 节点 reg 覆盖两个 pflash（pflash0 和 pflash1），
但 pflash0 是空的（固件槽位），CFI 探测会失败。**必须把 `flash@0` 改为只覆盖 pflash1**。

#### 修改前后对比

```diff
 flash@0 {
-    bank-width = <0x04>;
-    reg = <0x00 0x00 0x00 0x4000000 0x00 0x4000000 0x00 0x4000000>;
+    bank-width = <0x02>;
+    reg = <0x00 0x4000000 0x00 0x4000000>;
     compatible = "cfi-flash";
 };
```

**修改说明**：

| 字段 | 修改前 | 修改后 | 原因 |
|------|--------|--------|------|
| `reg` | 覆盖 pflash0(0x0) + pflash1(0x4000000) | 只覆盖 pflash1(0x4000000) | pflash0 是空固件槽位，CFI 探测失败会导致整个节点无法注册 |
| `bank-width` | `0x04` (32-bit) | `0x02` (16-bit) | QEMU pflash_cfi01 是 x16 宽度，0x04 会导致 `do_map_probe() failed` |

**reg 格式解析**（ARM64 `#address-cells=<2>`, `#size-cells=<2>`）：

```
修改前: <0x00 0x00         0x00 0x4000000   0x00 0x4000000   0x00 0x4000000>
         └─pflash0 起始───┘└─pflash0 大小──┘└─pflash1 起始──┘└─pflash1 大小──┘
         地址: 0x0           64MB              地址: 0x4000000    64MB

修改后: <0x00 0x4000000   0x00 0x4000000>
         └─pflash1 起始──┘└─pflash1 大小──┘
         地址: 0x4000000    64MB
```

#### 操作步骤

```bash
cd ~/workspace/qemu_virt_machine

# 1. 修改 reg：只覆盖 pflash1（地址 0x4000000，大小 64MB）
sed -i 's|reg = <0x00 0x00 0x00 0x4000000 0x00 0x4000000 0x00 0x4000000>;|reg = <0x00 0x4000000 0x00 0x4000000>;|' virt_custom.dts

# 2. 修改 bank-width：0x04(32-bit) → 0x02(16-bit)，匹配 QEMU pflash_cfi01 x16 宽度
sed -i 's|bank-width = <0x04>;|bank-width = <0x02>;|' virt_custom.dts

# 3. 重新编译 DTB
dtc -I dts -O dtb virt_custom.dts -o virt_custom.dtb
```

### 4.3 更新配置并重新编译

启用 CFI 后，Kconfig 会引入新的依赖项（`MTD_CFI_ADV_OPTIONS`、`MTD_CFI_INTELEXT`、
`MTD_PHYSMAP_COMPAT` 等），需用 `yes "" | make oldconfig` 一次性接受默认值：

```bash
cd ~/workspace/linux-4.9.263
yes "" | make ARCH=arm64 oldconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc)
```

### 4.4 验证配置

```bash
grep -E "CONFIG_MTD_CFI=|CONFIG_MTD_PHYSMAP=|CONFIG_MTD_BLOCK=" .config
```

## 5. 编译 libobject（aarch64）

```bash
./devops.sh build --platform=linux --arch=aarch64
```

编译产物位于 `sysroot/linux/aarch64/bin/xtools`，动态库位于 `sysroot/linux/aarch64/lib/`。

## 6. 部署代码到 QEMU（9p 共享目录）

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
  -drive file=/home/alan/workspace/qemu_virt_machine/flash.img,if=pflash,index=1,format=raw \
  -virtfs local,path=/home/alan/workspace/libobject/sysroot/linux/aarch64,mount_tag=host0,security_model=none,id=host0 \
  -virtfs local,path=/home/alan/workspace/libobject/tests/board/res,mount_tag=host1,security_model=none,id=host1 \
  -append "console=ttyAMA0 rdinit=/linuxrc"
```

> **说明**：
> - `host0`：共享 `sysroot/linux/aarch64`（bin/xtools + lib/）
> - `host1`：共享 `tests/board/res`（测试资源文件，如 `test_sq.img`）
> - `-virtfs` 必须在 QEMU 启动时就加上，否则 guest 中 mount 会报 `no channels available`

## 7. 运行 MTD 测试

### 7.1 手动测试

```sh
# 查看 MTD 设备信息
cat /proc/mtd

# 用 dd 读写 /dev/mtd0（需先擦除）
# 擦除（通过 mtd-utils 的 flash_erase，若 busybox 有）
flash_erase /dev/mtd0 0 0

# 写入
echo "hello mtd" > /tmp/test.txt
dd if=/tmp/test.txt of=/dev/mtd0 bs=1 count=10

# 读回
dd if=/dev/mtd0 bs=1 count=10
```

### 7.2 case 测试

进入 guest 后，先挂载 9p 共享目录：

```sh
mkdir -p /mnt /mnt/res
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host1 /mnt/res
export LD_LIBRARY_PATH=/mnt/lib
```

#### test_mtd（基础擦写读 + 越界保护）

```sh
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_mtd
```

测试通过时日志显示 `command suc, func_name = test_mtd`，实际输出：

```
[INFO]-[mtd open success, dev:/dev/mtd0, fd:6, size:0x2000000, erasesize:0x20000, ...]
[INFO]-[mtd open success, dev:/dev/mtd0, fd:6]
[INFO]-[mtd info: size:0x2000000, erasesize:0x20000, writesize:0x1, oobsize:0x0, type:0x3]
[INFO]-[mtd erase ok, offset:0x0, size:0x20000]
[INFO]-[erase ok, offset:0x0, size:0x20000]
[INFO]-[write ok, offset:0x0, size:0x100]
[INFO]-[read ok, offset:0x0, size:0x100]
[INFO]-[write/read verify ok, data matches]
[ERROR]-[mtd read failed, offset:0x2000000, size:0x100, errno:2(No such file or directory)]
[INFO]-[mtd close ok]
[VIP]-[command suc, func_name = test_mtd, ...]
```

> **关于最后的 ERROR 日志**：`offset:0x2000000`（设备末尾）越界读取是测试用例
> 故意设计的越界保护测试（`test_mtd.c` 第 81 行），期望 read 失败 → 测试通过。
> `errno:2` 是 `lseek` 超出设备边界时返回的错误码，属于预期行为。

#### test_mtd_write_file（分块大文件刷写）

```sh
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_mtd_write_file
```

生成 `2 × erasesize + 256` 字节的测试文件（跨 3 个擦除块），
通过 `write_file` 刷入 Flash 后校验头尾数据。

#### test_mtd_squashfs（文件系统挂载）

```sh
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_mtd_squashfs
```

流程：`write_file` 刷 [`test_sq.img`](../../tests/board/res/test_sq.img) →
`mount -t squashfs /dev/mtdblock0 /mnt/data` → 读文件验证 →
`umount`。

> **文件系统选择**：
> - **squashfs**（✅ 推荐）：只读，兼容性好，适合固件镜像。内核需 `CONFIG_SQUASHFS=y`。
> - **jffs2**（❌ QEMU virt 不兼容）：`jffs2_scan_medium` 在 x16 CFI NOR 上触发
>   `Unable to handle kernel paging request` 内核崩溃，Linux 4.9 已知问题。

## 8. MTD 类接口说明

libobject 的 MTD 类（`Mtd`）封装了 Linux MTD 子系统的访问，接口如下：

| 接口 | 说明 |
|------|------|
| `open(mtd, device)` | 打开 `/dev/mtdN`，获取设备信息（MEMGETINFO） |
| `close(mtd)` | 关闭设备 |
| `get_info(mtd, info)` | 获取设备信息（size/erasesize/writesize/oobsize/type） |
| `erase(mtd, offset, size)` | 按擦除块擦除（块对齐，MEMERASE） |
| `read(mtd, offset, buf, size)` | 读取数据（lseek + read） |
| `write(mtd, offset, buf, size)` | 写入数据（lseek + write，需先擦除） |
| `write_file(mtd, offset, filepath)` | 从文件路径读取数据，自动擦除并按块写入 MTD（内存仅占一个 erasesize） |

使用示例：

```c
#include <libobject/board/hal/mtd/Mtd.h>

Mtd *mtd = object_new(allocator, "Mtd", NULL);
mtd->open(mtd, "/dev/mtd0");

mtd_dev_info_t info;
mtd->get_info(mtd, &info);

/* 擦除第一个擦除块 */
mtd->erase(mtd, 0, info.erasesize);

/* 写入并读回 */
mtd->write(mtd, 0, buf, size);
mtd->read(mtd, 0, rbuf, size);

mtd->close(mtd);
object_destroy(mtd);
```

### write_file 示例

```c
/* 将 squashfs/jffs2 镜像刷入 Flash（自动擦除+分块写入） */
mtd->write_file(mtd, 0, "/mnt/res/test_sq.img");
```

> `write_file` 按 `erasesize` 分块读取文件，逐块写入，内存占用仅一个擦除块大小，适合大文件刷写。

## 9. 常见问题

### 9.1 guest 中没有 /dev/mtd0

- 确认内核已启用 `CONFIG_MTD`、`CONFIG_MTD_CFI`、`CONFIG_MTD_PHYSMAP_OF`。
- 确认 QEMU 启动参数包含 `-dtb virt_custom.dtb` 和 `-drive if=pflash,index=1`。
- 用 `dmesg \| grep -i mtd` 查看内核日志：
  - `do_map_probe() failed` → 设备树 `bank-width` 不对或 probe 了空的 pflash0。
  - 无任何 mtd 信息 → 内核未启用 `CONFIG_MTD_PHYSMAP_OF`。

### 9.2 QEMU 启动卡住（无 console 输出）

`-append "console=ttyAMA0"` 中的 `tty` 是**全小写**，写成大写 `TTY` 会导致
内核找不到 console 设备，启动看起来"卡住"。

### 9.3 9p 挂载失败（no channels available）

```
mount: mounting host0 on /mnt failed: No such file or directory
9pnet_virtio: no channels available for device host0
```

QEMU 启动时没加 `-virtfs` 参数。退出 QEMU（`Ctrl+A` 然后 `X`），
重新带 `-virtfs` 启动。

### 9.4 写入失败（write error）

MTD 写入前必须先擦除（NOR Flash 只能把 1 写成 0，不能把 0 写成 1）。
使用 `mtd->erase()` 擦除后再写入。

### 9.5 擦除失败（erase error）

擦除的 offset 和 size 必须按 `erasesize` 对齐，且不能超出设备大小。

### 9.6 测试日志中出现 ERROR（errno:2）

```
[ERROR]-[mtd read failed, offset:0x2000000, size:0x100, errno:2(No such file or directory)]
```

这是越界保护测试的预期行为（从设备末尾开始读取 → `lseek` 失败），
最终 `command suc` 表示测试通过，可以忽略这条 ERROR 日志。
