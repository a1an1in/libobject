# VFIO 用户态设备访问接口设计

> **快速回忆实现思路（8 步）**：
> ① 扫 `/sys/bus/pci/devices` 发现设备 → `readlink .../iommu_group` 拿 **group ID**
> ② 绑定 vfio-pci
> ③ 打开 **容器 → 组 → 设备** 三层 fd
> ④ mmap BAR 读寄存器
> ⑤ eventfd 绑中断
> ⑥ dma_map 映射 IOVA
> ⑦ 设备级 `dma_run` 搬运
> ⑧ 逆序析构（设备 → 组 → 容器）
>
> 详细流水线见下节 **2. 实现思路总览**。

## 1. 背景与目标

### 1.1 为什么需要 VFIO
现有 [`Uio_Pcie`](../src/board/hal/uio/Uio_Pcie.c) 基于 UIO + `/dev/mem`，能做的只是**寄存器访问**（MMIO），
**不支持 DMA**，且 `/dev/mem` 无 IOMMU 隔离。

VFIO（Virtual Function I/O）是内核提供的**带 IOMMU 隔离的用户态设备直通**框架：
- **寄存器访问**：设备 fd 上按 region offset mmap（IOMMU 隔离，替代 `/dev/mem`）。
- **中断**：`VFIO_DEVICE_SET_IRQS` + eventfd，用户态用 eventfd 收中断。
- **DMA**：`VFIO_IOMMU_MAP_DMA` 把用户缓冲映射为 IOVA，设备经 IOMMU 访问——这是 UIO 做不到的。

### 1.2 与现有类的对比

| | Uio_Pcie（现有） | Vfio_Pcie（本设计） |
|---|---|---|
| BAR 映射 | sysfs 读物理地址 + mmap `/dev/mem` | 设备 fd 按 `vfio_region_info.offset` mmap |
| 中断 | `/dev/uioX`（uio_pci_generic） | eventfd + io_worker |
| DMA | 不支持 | `VFIO_IOMMU_MAP_DMA` |
| 隔离 | 无（/dev/mem） | IOMMU group 隔离 |
| 依赖 | UIO + CONFIG_DEVMEM | IOMMU + vfio-pci 驱动 |

## 2. 实现思路总览

> 这一章把 VFIO 用户态驱动的**实现流水线**完整串一遍：从设备发现、拿 group ID，到打开三层 fd、
> mmap BAR、eventfd 中断、dma_map、设备级搬运、逆序析构。忘细节时先看这里，再回查
> 第 6（核心实现要点）、10（实现说明）、13（FAQ）章。

### 2.1 一句话

把 PCIe 设备**安全地交给用户态**：发现设备 → 绑定 vfio-pci → 拿容器/组/设备三层 fd →
mmap 寄存器（IOMMU 隔离）→ eventfd 收中断 → dma_map 做 DMA。全程不用写内核驱动。

### 2.2 类划分（谁负责什么）

- **`Vfio`**：与总线无关的通用机制——三层 fd 生命周期、region 信息/mmap、eventfd 中断、dma_map；
  只**声明**设备级 `dma_run`（默认 -1，供多态）。主机内存端的准备统一走 `dma_map`/`dma_unmap`。
- **`Vfio_Pcie`**：PCIe 语义——按 vendor/device 发现、iommu_group 解析、绑定 vfio-pci、BAR=region、
  偏移编码寄存器访问；并暴露 `dma_*` 入参块（由调用方填充，供 `dma_run` 读取）。
- **`Vfio_Pcie_Edu`**：具体设备（edu）——override `dma_run`，读入参块用 edu 寄存器做两段中转 + 中断完成。

### 2.3 完整流水线（对应真实代码函数）

#### 阶段 1：发现设备 + 拿 group ID（[`Vfio_Pcie.c`](../src/board/hal/vfio/Vfio_Pcie.c) `__open_device` 等）

1. 按 vendor/device 遍历 `/sys/bus/pci/devices`，`readdir` 找到匹配 BDF（如 `0000:00:03.0`）。
2. BAR 大小不再单独读 sysfs：`map_region` 用 `VFIO_DEVICE_GET_REGION_INFO`（region size）即可。
3. [`__get_group_path`](../src/board/hal/vfio/Vfio_Pcie.c:138)：`readlink /sys/bus/pci/devices/<bdf>/iommu_group`，
   取链接目标**最后一段数字** = group ID N。
4. [`__bind_to_vfio`](../src/board/hal/vfio/Vfio_Pcie.c:96)：`echo <BDF> > uio_pci_generic/unbind`（解绑旧驱动）→
   `echo <vendor> <device> > vfio-pci/new_id` → 必要时 `.../vfio-pci/bind`。
   绑定后内核才在 `/sys/class/vfio/<N>/dev` 有设备号，`/dev/vfio/<N>` 才可用。
5. 组装 `group_path = "/dev/vfio/<N>"`、`device_name = "<BDF>"`，调父类 `Vfio.__open`。

#### 阶段 2：打开三层 fd（[`Vfio.c`](../src/board/hal/vfio/Vfio.c) `__open`）

1. `container_fd = open("/dev/vfio/vfio")`；
2. `VFIO_GET_API_VERSION` 校验、`VFIO_CHECK_EXTENSION(VFIO_TYPE1_IOMMU)` 确认 TYPE1；
3. [`__ensure_group_devnode`](../src/board/hal/vfio/Vfio.c:49)：若 `/dev/vfio/<N>` 缺失，读
   `/sys/class/vfio/<N>/dev` 后 `mknod` 兜底；
4. `group_fd = open("/dev/vfio/<N>")`；
5. `VFIO_GROUP_SET_CONTAINER(group_fd, container_fd)`：组挂到容器（一组成一容器）；
6. `VFIO_SET_IOMMU(container_fd, VFIO_TYPE1_IOMMU)`：容器设为 TYPE1 域；
7. `device_fd = VFIO_GROUP_GET_DEVICE_FD(group_fd, "<BDF>")`：匿名设备 fd；
8. `VFIO_DEVICE_GET_INFO` → 存 `vfio->info`（num_regions / num_irqs）。

> 错误处理：任一步失败按"设备→组→容器"逆序 close。

#### 阶段 3：映射 BAR/region（[`__map_region`](../src/board/hal/vfio/Vfio.c:261)，BAR n = region n）

- `VFIO_DEVICE_GET_REGION_INFO(index)` → `reg.offset/size/flags`；
- 若 `flags & VFIO_REGION_INFO_FLAG_MMAP`，在**设备 fd** 上 `mmap(reg.size, MAP_SHARED, reg.offset)`；
- BAR n = region n（VFIO-PCI 约定），映射 BAR 直接用父类 `map_region(bar)`（已无独立 map_bar），无独立地址编码；
- 寄存器访问统一用父类 [`region_read/region_write`](../src/board/hal/vfio/Vfio.c:342)（index=region/BAR 号、region 内偏移；位宽用 `set_width` 配置，默认 32）。

#### 阶段 4：中断（[`__register_irq`](../src/board/hal/vfio/Vfio.c:384)）

1. `eventfd(0, EFD_NONBLOCK)`；
2. `VFIO_DEVICE_SET_IRQS(DATA_EVENTFD|ACTION_TRIGGER, index=irq_index, start=sub_index, count=1, data=efd)`
   把该向量绑到 eventfd（INTx=0 / MSI=1 / MSI-X=2）；
3. `io_worker` 监听 eventfd `EV_READ|EV_PERSIST`，中断来 → [`__irq_ev_callback`](../src/board/hal/vfio/Vfio.c:353)
   读计数 → 异步调 handler；
4. 每个 (index, 向量) 一个 `vfio_irq_ctx`（efd/worker/handler/opaque 合一）存进
   `irq_ctx[VFIO_MAX_IRQ_GROUPS][VFIO_MAX_IRQ_VECTORS_PER_GROUP]`；
5. INTx（AUTOMASKED 电平）需在 handler 里 `unmask_irq` 才收下一次；MSI 边沿触发无需。

#### 阶段 5：DMA（[`__dma_map`](../src/board/hal/vfio/Vfio.c:537) / [`__dma_unmap`](../src/board/hal/vfio/Vfio.c:590)）

- `VFIO_IOMMU_MAP_DMA(container_fd, {vaddr, iova, size})`：把**用户缓冲 VA** 与**自选 IOVA**
  一起交给内核，建立 `IOVA → 物理页(PA)` 映射并 pin 住物理页（**IOVA 是输入，不是翻译结果**，
  详见 6.4.1）；IOVA 从内部游标 `vfio->iova` 起按 4K 递增分配，**起点优先用 `vfio->iova_base`**
  （调用方/子类可设，未设置则取默认安全值 `VFIO_IOVA_BASE_DEFAULT = 0x0F000000`，避开低地址保留区）；
- `VFIO_IOMMU_UNMAP_DMA(container_fd, iova, size)` 解除。

#### 阶段 6：设备级搬运（[`Vfio.__dma_map`](../src/board/hal/vfio/Vfio.c:528) → 填 `dma_*` 入参块 → [`Vfio_Pcie_Edu.__dma_run`](../src/board/hal/vfio/Vfio_Pcie_Edu.c:116) → [`Vfio.__dma_unmap`](../src/board/hal/vfio/Vfio.c:580)）

库层不再提供 `dma_config`/`dma_copy`，只保留通用 `dma_map`/`dma_unmap` + 设备相关 `dma_run`：

- 准备（CPU/IOMMU）：调用方 `dma_map(buf_src) → iova_src`、`dma_map(buf_dst) → iova_dst`；
- 投递（入参块）：把 `iova_src/iova_dst/len` 写入 `pcie->dma_src/dma_dst/dma_len`；
- `dma_run()`：由具体设备 override（edu 读入参块做两段中转 guest→dma_buf→guest），触发时 CMD 带
  `EDU_DMA_IRQ`，用**中断完成**（[`__edu_dma_irq_handler`](../src/board/hal/vfio/Vfio_Pcie_Edu.c:50) 清中断 →
  置 volatile `dma_done`，[`__wait_dma_done`](../src/board/hal/vfio/Vfio_Pcie_Edu.c:77) 轮询）；
- 清理：调用方 `dma_unmap(iova_src/iova_dst)`（或在 `dma_src_va/dma_dst_va` 标记后由析构自动解除）。

#### 阶段 7：析构（[`Vfio_Pcie.__deconstruct`](../src/board/hal/vfio/Vfio_Pcie.c:584) → [`Vfio.__deconstruct`](../src/board/hal/vfio/Vfio.c:703)，逆序）

- 先停中断（worker_destroy、关 eventfd）→ 自动 unmap 遗留 DMA（`dma_src_va/dma_dst_va` 非空才 unmap）→
  munmap region → close 设备/组/容器 fd → 销毁 mutex。
- 子类先析构、父类后析构，保证 unmap DMA 时容器 fd 仍有效。

### 2.4 关键点（最容易忘的）

1. 三个 fd 顺序：**容器 → 组 → 设备**；释放**逆序**：设备 → 组 → 容器。
2. **group ID 是内核按 IOMMU 拓扑分配的**，不是自己选的；一设备固定属一个组。
3. 设备 fd 是 **GET_DEVICE_FD 动态拿的匿名 fd**，不是 `/dev` 下的设备节点。
4. 中断必须用 **eventfd**（device_fd 只能 ioctl，不能 read/poll 拿中断）。
5. MMIO 的 offset 是**设备 fd 地址空间内的 region 偏移**（VFIO 返回），不是物理地址。
6. IOVA 由**自己挑选**（并非由 VA 翻译而来），起点优先用 `vfio->iova_base`（未设置则用默认
   `0x0F000000` 高位窗口，避开低地址保留区）；映射完成后设备用 IOVA，由 IOMMU 翻译成物理地址。
7. `dma_run` 是设备相关唯一接口，藏在具体设备类 override；上层只依赖通用签名。

## 3. VFIO 用户态 API（Linux 4.9 uapi）

### 3.1 三层 fd
```
container = open("/dev/vfio/vfio")
group     = open("/dev/vfio/<N>")   // N 来自 /sys/bus/pci/devices/<bdf>/iommu_group
dev_fd    = VFIO_GROUP_GET_DEVICE_FD(group, "0000:00:02.0")
```

### 3.2 ioctl 序列
```c
/* container */
VFIO_GET_API_VERSION          // _IO(VFIO_TYPE, +0)
VFIO_CHECK_EXTENSION(VFIO_TYPE1_IOMMU)  // _IO(+1)
/* group -> container */
VFIO_GROUP_SET_CONTAINER(group_fd, container_fd)  // _IO(+4, __s32)
VFIO_SET_IOMMU(container_fd, VFIO_TYPE1_IOMMU)     // _IO(+2, __s32)
/* device */
VFIO_GROUP_GET_DEVICE_FD(group_fd, "0000:00:02.0") // _IO(+6, char[])
VFIO_DEVICE_GET_INFO(dev_fd, &info)   // vfio_device_info{argsz,flags,num_regions,num_irqs}
VFIO_DEVICE_GET_REGION_INFO(dev_fd, &reg)  // vfio_region_info{argsz,flags,index,cap_offset,size,offset}
mmap(dev_fd, size, R|W, MAP_SHARED, reg.offset)   // region mmap
VFIO_DEVICE_SET_IRQS(dev_fd, &irqset)  // eventfd 中断
/* DMA */
VFIO_IOMMU_MAP_DMA(container_fd, &dma_map)     // 入参 (vaddr, iova, size)，建立 IOVA->PA 映射
VFIO_IOMMU_UNMAP_DMA(container_fd, &dma_unmap)
```

### 3.3 关键结构（[`vfio.h`](../linux-4.9.263/include/uapi/linux/vfio.h)）
```c
struct vfio_device_info {
    __u32 argsz; __u32 flags;   // VFIO_DEVICE_FLAGS_PCI(1<<1)
    __u32 num_regions; __u32 num_irqs;
};
struct vfio_region_info {
    __u32 argsz; __u32 flags;   // VFIO_REGION_INFO_FLAG_MMAP(1<<2)
    __u32 index; __u32 cap_offset;
    __u64 size;  __u64 offset;  // offset = 设备 fd 上 mmap 的偏移
};
struct vfio_irq_info {
    __u32 argsz; __u32 flags;   // EVENTFD(1<<0)/MASKABLE(1<<1)/AUTOMASKED(1<<2)/NORESIZE(1<<3)
    __u32 index; __u32 count;
};
struct vfio_irq_set {
    __u32 argsz; __u32 flags;   // DATA_EVENTFD(1<<2)|ACTION_TRIGGER(1<<5)
    __u32 index; __u32 start; __u32 count; __s8 data[];  // data = eventfd 数组
};
struct vfio_iommu_type1_dma_map {
    __u32 argsz; __u32 flags;   // VFIO_DMA_MAP_FLAG_READ(1<<0)|WRITE(1<<1)
    __u64 vaddr; __u64 iova; __u64 size;
};
struct vfio_iommu_type1_dma_unmap {
    __u32 argsz; __u32 flags; __u64 iova; __u64 size;
};
```

### 3.4 PCI 固定 region/irq 映射（vfio-pci）
```c
enum { /* region index */
    VFIO_PCI_BAR0_REGION_INDEX, ... VFIO_PCI_BAR5_REGION_INDEX,  // 0-5 = BAR
    VFIO_PCI_ROM_REGION_INDEX, VFIO_PCI_CONFIG_REGION_INDEX,     // 6,7
    VFIO_PCI_VGA_REGION_INDEX, VFIO_PCI_NUM_REGIONS = 9
};
enum { /* irq index */
    VFIO_PCI_INTX_IRQ_INDEX, VFIO_PCI_MSI_IRQ_INDEX,
    VFIO_PCI_MSIX_IRQ_INDEX, VFIO_PCI_ERR_IRQ_INDEX, VFIO_PCI_REQ_IRQ_INDEX
};
```
- BAR0-5 即 region 0-5；未实现的 region 返回 `size == 0`。
- 中断按 index：INTX(0)/MSI(1)/MSIX(2)；`count` 为向量数（MSI/MSI-X 可多个 subindex）。

## 4. 类层次设计

```
Obj
 └── Vfio               通用 VFIO 基类（container/group/device/region/irq/dma，与总线无关）
      └── Vfio_Pcie     PCIe 设备（发现、BAR=region、偏移编码寄存器访问、INTx/MSI 中断）
           └── Vfio_Pcie_Edu  edu 教学设备（QEMU）：override dma_run（两段中转 DMA，中断完成）
```

- `Vfio` 负责**通用机制**：三层 fd 生命周期、region 信息/mmap、eventfd 中断、DMA 映射，
  并**声明**设备级 DMA 搬运接口 `dma_run`（默认返回 -1 不支持，供多态）。
- `Vfio_Pcie` 负责**PCIe 语义**：按 vendor/device 发现、`iommu_group` 解析、BAR=region、
  寄存器访问复用偏移编码（高位 region、低位偏移，与 `Uio_Pcie.pcie_bar_addr` 一致）；
  并暴露 `dma_*` 入参块（由调用方填充，供 `dma_run` 读取）。
- `Vfio_Pcie_Edu` 负责**具体设备**（edu）：override `dma_run`，读入参块用 edu 寄存器做两段中转搬运。

## 5. API 设计

### 5.1 Vfio（基类，`src/include/libobject/board/hal/vfio/Vfio.h`）
```c
typedef struct vfio_dev_info {
    uint32_t flags;        /* VFIO_DEVICE_FLAGS_* */
    uint32_t num_regions;  /* region 数量 */
    uint32_t num_irqs;     /* irq 组数量 */
    char group_path[64];   /* /dev/vfio/N */
    char device_name[32];  /* 如 "0000:00:02.0" */
} vfio_dev_info_t;

typedef struct vfio_region_info_ex {
    int      index;        /* region 索引（PCIe: BAR0-5） */
    uint64_t offset;       /* 设备 fd mmap 偏移（内核返回） */
    uint64_t size;         /* 大小 */
    uint32_t flags;        /* VFIO_REGION_INFO_FLAG_* */
} vfio_region_info_t;

struct Vfio_s {
    Obj parent;
    int (*construct)(Vfio *, char *);
    int (*deconstruct)(Vfio *);
    int (*set)(Vfio *, char *, void *);
    void *(*get)(Vfio *, char *);
    char *(*to_json)(Vfio *);

    /* 生命周期：打开 iommu 组 + 设备 */
    int (*open)(Vfio *vfio, char *group_path, char *device_name);
    int (*close)(Vfio *vfio);
    /* 信息 */
    int (*get_info)(Vfio *vfio, vfio_dev_info_t *info);
    int (*get_region_info)(Vfio *vfio, int index, vfio_region_info_t *info);
    /* region mmap */
    int (*map_region)(Vfio *vfio, int index);   /* 多 region 并存，region_base[] */
    int (*unmap_region)(Vfio *vfio, int index);
    /* 配置 region 访问位宽：32/64（默认 32，设备相关） */
    int (*set_width)(Vfio *vfio, int width);
    /* 通用 region 访问：index/offset；位宽用 set_width 配置的 reg_width */
    int (*region_read)(Vfio *vfio, int index, uint64_t offset, uint64_t *data);
    int (*region_write)(Vfio *vfio, int index, uint64_t offset, uint64_t data);
    /* 中断：eventfd + io_worker 异步 */
    int (*register_irq)(Vfio *vfio, int irq_index, int sub_index,
                        vfio_irq_handler_t handler, void *opaque);
    int (*mask_irq)(Vfio *vfio, int irq_index, int sub_index);
    int (*unmask_irq)(Vfio *vfio, int irq_index, int sub_index);
    /* DMA：把用户缓冲映射为 IOVA */
    int (*dma_map)(Vfio *vfio, void *buf, uint64_t size, uint64_t *iova);
    int (*dma_unmap)(Vfio *vfio, uint64_t iova, uint64_t size);
    /* 设备级 DMA 搬运接口（多态：Vfio 只声明，默认返回 -1 不支持；
     * 主机内存端由调用方 dma_map 准备，dma_run 由具体设备类如 Vfio_Pcie_Edu override） */
    int (*dma_run)(Vfio *vfio);

    /*attribs*/
    int container_fd;
    int group_fd;
    int device_fd;
    char *group_path;
    char device_name[32];
    vfio_dev_info_t info;
    uint8_t *region_base[16];       /* 各 region mmap 基址 */
    uint64_t region_size[16];
    vfio_irq_ctx_t irq_ctx[VFIO_MAX_IRQ_GROUPS][VFIO_MAX_IRQ_VECTORS_PER_GROUP]; /* 每向量中断状态（efd/worker/handler/opaque 合一） */
    uint32_t irq_count;              /* 最近一次中断计数（供 handler 读取） */
    pthread_mutex_t lock;            /* 进程内互斥锁，保护所有操作 */
    uint64_t iova;                   /* 内部 IOVA 分配游标（dma_map 按页递增分配） */
};
```

### 5.2 Vfio_Pcie（`src/include/libobject/board/hal/vfio/Vfio_Pcie.h`）
```c
struct Vfio_Pcie_s {
    Vfio parent;
    /* 按 vendor/device 发现 + 打开（sysfs → iommu_group → 父类 open）；
     * 映射/访问 BAR 用父类 region 接口（BAR n = region n） */
    int (*open_device)(Vfio_Pcie *p, uint32_t vendor, uint32_t device);
    /* 设备级 DMA：dma_run（接口，默认继承 Vfio 不支持，由具体设备类如
     * Vfio_Pcie_Edu override）；主机内存端由调用方 dma_map 准备，并写入 dma_* 入参块 */
    int (*dma_run)(Vfio_Pcie *p);
    /*attribs*/
    char *bdf;
    /* DMA 搬运入参块（调用方填充，dma_run 读取执行） */
    void *dma_src_va;               /* 已映射的源缓冲 VA（标记后析构自动 unmap） */
    void *dma_dst_va;               /* 已映射的目的缓冲 VA（标记后析构自动 unmap） */
    uint64_t dma_src;               /* 源 IOVA */
    uint64_t dma_dst;               /* 目的 IOVA */
    uint32_t dma_len;               /* 长度（字节） */
    int dma_dir;                    /* 抽象方向：VFIO_DMA_TO_DEVICE/FROM_DEVICE */
};
```
> 说明：`Vfio_Pcie` 不继承 `Uio_Pcie`（两者后端不同：VFIO vs UIO+/dev/mem）。
> VFIO 侧把 BAR 归一到 region，寄存器访问统一走父类 `region_read/region_write(index, offset, width)`，
> 不再有独立的偏移编码 / `bar_shift`。

## 6. 核心实现要点

### 6.1 open（三层 fd）
```c
__open(vfio, group_path, device_name):
  1. container_fd = open("/dev/vfio/vfio", O_RDWR);
  2. VFIO_GET_API_VERSION / VFIO_CHECK_EXTENSION(VFIO_TYPE1_IOMMU)
  3. __ensure_group_devnode(group_path)   // /dev/vfio/<N> 缺失时按 /sys/class/vfio/<N>/dev mknod
  4. group_fd = open(group_path, O_RDWR)   // "/dev/vfio/<N>"
  5. VFIO_GROUP_SET_CONTAINER(group_fd, container_fd)
  6. VFIO_SET_IOMMU(container_fd, VFIO_TYPE1_IOMMU)
  7. device_fd = ioctl(group_fd, VFIO_GROUP_GET_DEVICE_FD, device_name)
  8. VFIO_DEVICE_GET_INFO → vfio->info
```
错误处理：每步失败 `goto`/THROW 前先释放已开 fd（容器→组→设备 逆序 close）。

### 6.2 map_region（BAR 映射）
```c
__map_region(vfio, index):
  reg.argsz = sizeof(reg); reg.index = index;
  ioctl(device_fd, VFIO_DEVICE_GET_REGION_INFO, &reg);
  if (!(reg.flags & VFIO_REGION_INFO_FLAG_MMAP)) return -1;
  mmap(NULL, reg.size, PROT_READ|PROT_WRITE, MAP_SHARED,
       device_fd, reg.offset);          // 关键：用 reg.offset（非物理地址）
  vfio->region_base[index] = base; vfio->region_size[index] = reg.size;
```
> 与 `/dev/mem` 的区别：offset 是**设备 fd 地址空间内的 region 偏移**（VFIO 返回），
> 不是物理地址；由 IOMMU 隔离。

### 6.3 register_irq（eventfd + io_worker）
```c
__register_irq(vfio, irq_index, sub_index, handler, opaque):
  efd = eventfd(0, EFD_NONBLOCK);          // 创建 eventfd
  irqset = { .argsz, .flags = DATA_EVENTFD|ACTION_TRIGGER,
             .index = irq_index, .start = sub_index, .count = 1,
             .data = &efd };
  ioctl(device_fd, VFIO_DEVICE_SET_IRQS, &irqset);   // 绑定中断→eventfd
  worker = io_worker(allocator, efd, EV_READ|EV_PERSIST,
                     NULL, NULL, __irq_ev_callback, handler, vfio);
```
`__irq_ev_callback`：`read(efd, &cnt, 8)` 清事件 → 调 `worker->work_callback`（handler）。
（与 Uio.register_irq / Gpio.register_event 同一套 io_worker 模式。）
> AUTOMASKED 中断（电平触发）需要在 handler 里 `unmask_irq` 才能收下一次。

### 6.4 DMA
```c
__dma_map(vfio, buf, size, *iova):
  /* ① 先挑一个「设备侧虚拟地址」（IOVA）：内部游标按页递增分配 */
  if (vfio->iova == 0)                               /* 起点优先 iova_base，未设置用默认高位窗口 */
      vfio->iova = vfio->iova_base ? vfio->iova_base : 0x0F000000ULL;
  base = (vfio->iova + PAGE - 1) & ~(PAGE - 1);
  /* ② 把 (VA, IOVA) 一起交给内核：建立 IOVA->PA 映射并 pin 住物理页 */
  dma = { .argsz, .flags = READ|WRITE, .vaddr = (uint64_t)buf, .iova = base, .size };
  ioctl(container_fd, VFIO_IOMMU_MAP_DMA, &dma);
  *iova = dma.iova;           /* 回传：即上面挑选的 base，设备用它发 DMA */
  vfio->iova = base + size;   /* 游标前进，下次分配新地址 */
__dma_unmap(vfio, iova, size):
  un = { .argsz, .iova, .size };   /* 解映射只能用 IOVA（IOMMU 里登记的就是它） */
  ioctl(container_fd, VFIO_IOMMU_UNMAP_DMA, &un);
```
> IOVA 分配：本实现用内部游标 `vfio->iova` 按页（4K）递增；**起点优先用 `vfio->iova_base`**
> （调用方/子类可设）；未设置时取默认安全值 `VFIO_IOVA_BASE_DEFAULT = 0x0F000000（240MB）`
> （避开低地址保留区/guest RAM 区，并为常见设备 dma_mask 留出空间）。IOVA 空间/位宽由设备
> (dma_mask) 与 IOMMU 决定，通用类不假设。

#### 6.4.1 地址模型：VA / PA / IOVA 三者关系（重点澄清）

DMA 里同时出现三种地址，最容易混淆，务必区分：

| 地址 | 全称 | 属于谁 | 谁认识它 | 说明 |
|------|------|--------|----------|------|
| **VA** | CPU 虚拟地址 | 进程 | MMU | `dma_map` 的入参 `buf` 就是 VA；进程私有，设备不认识 |
| **PA** | 物理地址 | 内存 | 内存控制器 | VA 与 IOVA 最终都翻译到它 |
| **IOVA** | I/O 虚拟地址（设备侧虚拟地址） | 设备 | IOMMU | 设备 DMA 寄存器里填的就是它 |

**五条关键结论（易错点）：**

1. **VA 不能直接给设备**：设备 DMA 寄存器不认 CPU 的 VA；把 VA 填进去会被当成总线地址，
   翻译不到期望内存，甚至触发 IOMMU fault（DMAR fault / SMMU abort）。
2. **IOVA 不是"把 VA 翻译得到的"**：IOVA 是**独立的 I/O 地址空间**，由驱动/内核
   **自行挑选或分配**，与 VA 的数值没有换算关系。三者的正确关系是"两条路汇聚到同一块 PA"：

       VA  --(MMU 页表)-->  PA  <--(IOMMU 页表)--  IOVA

   即 **VA 与 IOVA 是平级的两个虚拟地址**，各自翻译到同一块物理内存（`buf`），
   而不是"VA → IOVA"的翻译关系。
3. **IOVA 是 dma_map 的"输入"，不是输出**：`VFIO_IOMMU_MAP_DMA` 接收 `(vaddr, iova, size)`
   —— `vaddr` 告诉内核"要映射哪块内存"，`iova` 是**调用方事先选好的设备侧地址**。
   ioctl 只负责"建立 `IOVA → PA` 映射表项并 pin 住物理页"，**不替调用方分配 IOVA**
   （因此 `*iova` 回传的就是传入值）。这正是本实现要维护 `vfio->iova` 游标的原因。
4. **翻译的终点一定是 PA**：IOMMU 页表项里存的是**输出物理地址（PFN）+ 权限位**；
   设备从不看页表，是 IOMMU 代替设备去查表；页表基址存在 IOMMU 硬件寄存器里，不属于 IOVA。
5. **无 IOMMU 时**：总线地址就等于物理地址（直通/无转换），此时 DMA 寄存器要填 PA，
   "IOVA"这个概念不存在。本类依赖 `CONFIG_VFIO_IOMMU_TYPE1`，**必有 IOMMU**，
   所以这里的"总线地址"恒为 IOVA。

**本实现的完整数据流：**

```
   CPU 侧   buf(VA) ──(MMU)──► PA ─┐
                                   │ 同一块物理内存
   VFIO 侧  dma_map(buf, size, &iova) ── 建立 IOVA→PA 映射、pin 物理页
                                   │
   设备侧   DMA 寄存器 := IOVA ──(IOMMU 翻译)──► PA ──► 读写 buf 的真实物理页
```

> 一句话记忆：**VA 和 IOVA 是两个门牌，指向同一栋房子（PA）；IOVA 是给设备的门牌，
> 由你挑选后连同 VA 一起交给 IOMMU 登记，而不是从 VA 换算来的。**

#### 6.4.2 一次 DMA 的两端地址：谁提供、谁投递（常见误区澄清）

> **常见误区**：以为"做 DMA 要给外设配置它在总线上的虚拟地址"、或者"把外设 BAR 的地址
> 填进 `dma_map` 就能让数据搬进外设"。**这两点都是错的**——见下。

一次 DMA 要访问两个端点，但**两个端点的地址来源与投递方式并不相同**：

| 端点 | 地址从哪来 | 需要 `dma_map` 吗 | 怎么交给设备 |
|------|-----------|------------------|-------------|
| **主机内存** | 你 `dma_map` 产出的 IOVA | ✅ 需要 | 写进设备 DMA 寄存器（MMIO） |
| **设备自身**（内部 FIFO/缓冲，隐含端） | 无地址，设备内部固定 | ❌ 不需要 | 不用配（设备自己知道） |
| **另一个可寻址端点**（另一块内存 / 对端 BAR） | 内存→IOVA；BAR→其总线物理地址 | 内存要 map；**BAR 不能 map** | 写进设备 DMA 寄存器（MMIO） |

**两阶段模型（准备 + 投递），不是"两端各管各的"：**

```
① 准备（VFIO / IOMMU）：dma_map(VA) ──► 产出 IOVA，并在 IOMMU 建 IOVA→PA，
                        使这块主机内存可被设备 DMA 访问
② 投递（MMIO 寄存器）：把设备需要访问的地址（内存端 IOVA；若有第二端点则还有它的地址）
                        写进设备 DMA 寄存器（SRC/DST/CNT…）
③ 触发（MMIO 寄存器）：写 DMA 启动位 ──► 设备作为 master 按这些地址搬运
```

**五条要点：**

1. **VFIO 和 MMIO 不是"各管一端"**，而是同一条链路的"**准备**"与"**投递**"两步，都服务于
   "让设备访问那块内存"。
2. **`dma_map` 只负责主机内存端**：它建立 `IOVA→PA` 映射，但**不会把 IOVA 告诉设备**——
   投递必须靠 MMIO 寄存器（如 `dma_run` 里写 `SRC/DST/CNT`）。
3. **"设备端地址通过寄存器配置"要修正**：通过寄存器配的通常是**内存端的 IOVA**；
   **设备自身端一般没有地址、不配置**。以 edu 为例，`dma_run` 写 `EDU_DMA_SRC/EDU_DMA_DST`
   填的都是**内存端 IOVA**（见 [`Vfio_Pcie_Edu.c`](../src/board/hal/vfio/Vfio_Pcie_Edu.c:146)）。
4. **不要把外设 BAR 的地址填进 `dma_map.vaddr`**：BAR 是 MMIO（VMA 带 `VM_IO|VM_PFNMAP`），
   没有可 pin 的页，`VFIO_IOMMU_MAP_DMA` 会失败。要 DMA 到 BAR / 对端设备只能走 **P2P**：
   把对端 **BAR 的总线物理地址**直接写进设备 DMA 目标寄存器（需拓扑/IOMMU 支持），
   **不经过 `dma_map`**。
5. **"只配一端"何时成立**：当另一端就是设备自身（隐含端）时，CPU 只需 `dma_map` 内存端并把 IOVA
   投递给设备；若两端都要显式地址（如 edu 的复制机模型），则两端都要 map、都要投递。

**三种典型场景：**

```
A. 单端（设备端隐含，真设备读/写内存）
   dma_map(RAM) → iova ──[MMIO]──► 设备寄存器 SRC := iova
   设备自己 DMA 读内存进内部（设备端无地址，不配）

B. 两端都是内存（edu 复制机）
   dma_map(src) → iova_s ──[MMIO]──► SRC := iova_s
   dma_map(dst) → iova_d ──[MMIO]──► DST := iova_d

C. 一端内存 + 一端外设 BAR（P2P，当前实现不支持）
   dma_map(RAM) → iova  ──[MMIO]──► 设备寄存器 := iova       （内存端 IOVA）
   对端 BAR 总线物理地址 ──[MMIO]──► 设备寄存器 := peer_bar   （不 map）
```

> 一句话：**`dma_map` 准备内存端的 IOVA，MMIO 把地址"投递"给设备；设备端若是自身则无地址可配。**
> **不需要、也不能给 `dma_map` 配置"外设的总线虚拟地址"。**

> 与当前实现的关系：库层已**移除** `dma_config`/`dma_copy`，只保留通用 `dma_map`/`dma_unmap` +
> 设备相关 `dma_run`。三种场景统一表达为：
>   A. 单端（设备端隐含）：`dma_map(内存)` 得到 IOVA，填 `dma_src`，调 `dma_run`；
>   B. 两端都是内存（edu 复制机）：`dma_map ×2`，填 `dma_src/dma_dst`，调 `dma_run`；
>   C. 一端内存 + 一端外设 BAR（P2P）：`dma_map` 只 map 内存端，BAR 端直接填其**总线物理地址**。
> "设备端配置"仍由具体设备类的 `dma_run` 决定（库不预设），映射/解映射由调用方负责
> （或在 `dma_src_va/dma_dst_va` 标记后由 `Vfio_Pcie.__deconstruct` 自动解除）。

### 6.5 设备级 DMA 搬运接口（dma_run）
```c
/* 抽象方向：VFIO_DMA_TO_DEVICE（设备读 guest）/ VFIO_DMA_FROM_DEVICE（设备写 guest） */
int (*dma_run)(Vfio *vfio);   /* 读 Vfio_Pcie.dma_* 入参块，触发并等待完成（同步阻塞） */
```
- **分层**：库层**只保留通用的 `dma_map`/`dma_unmap`**（CPU/IOMMU 侧）与设备相关的
  `dma_run`（`Vfio` 声明，默认返回 -1；`Vfio_Pcie_Edu` override）。原先"两端都是内存"的
  `dma_config`/`dma_copy` 已移除——它们只适配 edu 复制机模型，不通用。
- **调用方职责**：先 `dma_map(VA) → IOVA`，把 IOVA/长度写入 `pcie->dma_src/dma_dst/dma_len`
  入参块，再调 `dma_run`；最后自己 `dma_unmap`。
  `Vfio_Pcie_Edu` override `dma_run`（edu 只能 guest↔dma_buf，故两段中转）。
- **中断完成（无锁）**：`Vfio_Pcie_Edu.dma_run` 触发时 CMD 置 `EDU_DMA_IRQ(0x4)`，
  用 `register_irq`（eventfd + io_worker）把 edu 的 DMA_IRQ 绑到中断处理函数；
  处理函数完成**中断应答**：清设备中断（写 0x64）→ `unmask_irq`（AUTOMASKED 电平
  触发，重新使能）→ 置 **volatile 完成标志**（无需额外 eventfd，中断已经走 VFIO
  的 irq_efd 到达）；`dma_run` 轮询该标志（带超时），替代轮询设备寄存器。
- **中断方式可切换（`dma_irq_mode`）**：`Vfio_Pcie_Edu` 暴露
  `dma_irq_mode = EDU_IRQ_INTX(0) / EDU_IRQ_MSI(1)`（默认 INTx），`dma_run` 据此选
  `register_irq` 的 `irq_index`（INTx→0 / MSI→1，向量 0）；中断处理函数里 **只有
  INTx 需要 `unmask_irq`**（电平触发 AUTOMASKED），MSI 是边沿触发、无需 unmask。
  edu 硬件是单向量 MSI（`msi_init(pdev, 0, 1, ...)`，无 MSI-X）。
- **MSI 在 QEMU TCG 下的投递限制（实测）**：`test_vfio_dma` 用 MSI 方式实测——
  `register_irq(index:1)` 成功（guest vfio-pci 已分配 LPI 并启用设备 MSI）、DMA 完成
  后读 `irq_status(0x24)=0x100`（设备侧 `edu_raise_irq` 已置 DMA_IRQ 并 `msi_notify`），
  但 **guest 一直收不到该 LPI** → `dma_run` 超时。结论：代码路径正确（INTx 中断完成
  正常），断点位于 **QEMU TCG 的 MSI→ITS→LPI 投递**（本环境 QEMU v11.1 已启用
  `tcg_its`，`virt.c:2934` AUTO→ITS，`create_its()` 正常创建）。
- **设备差异封装**：`dma_run` 触发方式每台设备不同（写 CMD 位/doorbell/描述符环），
  全部藏在具体设备类的 override 中；上层只依赖通用签名。
- **可选自动清理**：若调用方在 `dma_src_va/dma_dst_va` 里标记了已映射缓冲，
  `Vfio_Pcie.__deconstruct` 会自动 `dma_unmap`（析构顺序子类先、父类后，此时容器 fd
  仍有效）；未标记则由调用方显式 unmap。

## 7. 错误处理与线程安全

- **统一 TRY/CATCH/FINALLY**（代码库风格），每个 op 里 `pthread_mutex_lock` 放 TRY 内、
  `unlock` 放 FINALLY 内（`locked` 标志防误解锁），与 [`Gpio.c`](../src/board/hal/gpio/Gpio.c) 一致。
- **返回值**：成功经 CATCH 返回 1（代码库惯例，调用方用 `< 0` 判失败）；失败返回负 errno。
- **逐步回滚**：open 里任一步失败按"设备→组→容器"逆序 close；map_region 失败 `munmap`；
  register_irq 失败关 eventfd + worker_destroy。
- **生命周期**：`deconstruct` 依次 `worker_destroy`、关 eventfd、`munmap` region、close
  设备/组/容器 fd、`pthread_mutex_destroy`。
- **DMA 与中断的释放顺序**：先停中断，再 unmap DMA，最后关容器。

## 8. 环境前提与验证

### 8.1 前提
- 平台必须有 **IOMMU**，设备落在某个 `iommu_group`
  （`/sys/bus/pci/devices/<bdf>/iommu_group` 存在）。
- 设备要绑 **vfio-pci**（不是 uio_pci_generic）：
  ```sh
  echo 1234 11e8 > /sys/bus/pci/drivers/vfio-pci/new_id   # edu
  echo 0000:00:02.0 > /sys/bus/pci/drivers/uio_pci_generic/unbind
  echo 0000:00:02.0 > /sys/bus/pci/drivers/vfio-pci/bind
  ```
- 内核：`CONFIG_VFIO=y`、`CONFIG_VFIO_IOMMU_TYPE1=y`、`CONFIG_VFIO_PCI=y`
  （本套 4.9 配置已具备）。

### 8.2 QEMU 验证改造
当前 `-M virt` **默认无 IOMMU**（`VIRT_IOMMU_NONE`）→ 无 `iommu_group` → VFIO 打开组失败。
需：
- `-M virt,iommu=smmuv3`（SMMUv3 虚拟 IOMMU，virt.c 已支持该选项）；
- guest 内核 `CONFIG_ARM_SMMU_V3=y`；
- edu 绑 vfio-pci（8.1 步骤）；
- 确认 `/sys/bus/pci/devices/0000:00:02.0/iommu_group` 存在。

### 8.3 测试用例建议
- `test_vfio_pcie`：open_device(edu) → map_region(0) → 读 ID 寄存器（0x10000ed）→
  写读 addr4 往返（同 test_uio_pcie，但走 VFIO region mmap）。
- `test_vfio_dma`：`dma_map` 两段缓冲（src/dst）→ 触发 edu DMA 引擎做**两段式内存搬运**
  `buf_src →(FROM_PCI)→ edu dma_buf →(TO_PCI)→ buf_dst` → 轮询 RUN 位完成 →
  校验 `dst == src`（验证 IOMMU MAP_DMA 的 IOVA 读 + IOVA 写两条路径）。

## 9. 文件规划（已实现）
| 文件 | 说明 |
|------|------|
| `src/include/libobject/board/hal/vfio/Vfio.h` | Vfio 基类头文件（已实现） |
| `src/board/hal/vfio/Vfio.c` | Vfio 实现：三层 fd/open/close、get_info/get_region_info、map_region/unmap_region、register_irq（eventfd+io_worker）、mask/unmask_irq、dma_map/dma_unmap（IOVA 高位窗口游标） |
| `src/include/libobject/board/hal/vfio/Vfio_Pcie.h` | Vfio_Pcie 头文件（已实现） |
| `src/board/hal/vfio/Vfio_Pcie.c` | Vfio_Pcie 实现：open_device（sysfs 发现 + iommu_group 解析 + 绑定 vfio-pci）、`dma_*` 入参块（供 dma_run 使用）；映射/访问 BAR 走父类 region 接口 |
| `src/include/libobject/board/hal/vfio/Vfio_Pcie_Edu.h` | Vfio_Pcie_Edu 头文件（已实现）：继承 Vfio_Pcie，override dma_run |
| `src/board/hal/vfio/Vfio_Pcie_Edu.c` | Vfio_Pcie_Edu 实现：edu DMA 两段中转（guest→dma_buf→guest）触发，DMA 完成用中断（register_irq + 条件变量）等待（已实现） |
| `tests/board/test_vfio_dma.c` | 测试：edu DMA 引擎演示 —— `dma_map ×2` + 填 `dma_*` 入参块 + `dma_run` → 两段式 mem-to-mem 往返（guest mem → edu dma_buf → guest mem）校验，测试自行 `dma_unmap`（已实现） |
| `doc/board/vfio设计文档.md` | 本文档 |

> 构建：`src/board/CMakeLists.txt` 与 `tests/CMakeLists.txt` 都用 `file(GLOB_RECURSE)`，
> 新增源文件需重跑 `cmake .`（glob 在 configure 时求值）再 `make`；`make install` 更新 sysroot 的 xtools。

## 10. 实现说明

### 10.1 Vfio 基类（[`Vfio.c`](../src/board/hal/vfio/Vfio.c)）
- **open**：`/dev/vfio/vfio`（container）→ `VFIO_GET_API_VERSION`/`VFIO_CHECK_EXTENSION(TYPE1)` →
  `/dev/vfio/<N>`（group）→ `VFIO_GROUP_SET_CONTAINER` → `VFIO_SET_IOMMU(TYPE1)` →
  `VFIO_GROUP_GET_DEVICE_FD` → `VFIO_DEVICE_GET_INFO`。任一步失败按"设备→组→容器"逆序回滚。
- **map_region**：`VFIO_DEVICE_GET_REGION_INFO` 拿 `offset/size`，在**设备 fd** 上 mmap
  （`reg.offset` 是设备 fd 地址空间内的 region 偏移，非物理地址，IOMMU 隔离）。
- **register_irq**：eventfd + `VFIO_DEVICE_SET_IRQS(DATA_EVENTFD|ACTION_TRIGGER)` + io_worker
  异步（同 [`Uio.register_irq`](../src/board/hal/uio/Uio.c:399)）。`vfio_irq_set.data[]` 为柔性数组，
  用"内嵌 set + 尾随 `__s32`"的局部结构体避免堆分配（消除 memcpy 溢出警告）。
- **dma_map**：`VFIO_IOMMU_MAP_DMA`，IOVA 游标 `vfio->iova` 按页（4K）递增；**起点优先用**
  `vfio->iova_base`（未设置则默认 `0x0F000000` 高位窗口，避开低地址保留区/guest RAM）。

### 10.2 Vfio_Pcie 子类（[`Vfio_Pcie.c`](../src/board/hal/vfio/Vfio_Pcie.c)）
- **open_device**：按 vendor/device 扫 `/sys/bus/pci/devices` → 解析
  `/sys/bus/pci/devices/<BDF>/iommu_group`（symlink 末尾数字 → `/dev/vfio/<N>`）→
  绑定 vfio-pci（先解绑 uio_pci_generic，再写 `new_id`，必要时显式 bind）→ 调父类 `open`。
- **BAR 映射**：BAR n = region n（VFIO-PCI 约定），直接调用父类 `map_region(bar)`（已无独立 map_bar）；
  无独立地址编码，也不需 `bar_shift`/`bar_size[]`/`bar_mapped[]`。
- 寄存器访问统一用父类 `region_read`/`region_write`（index=region/BAR 号、region 内偏移；
  位宽用父类 `set_width` 配置——edu 在 construct 里 `set_width(32)`），
  在 **region mmap 基址**上做（非 `/dev/mem`），越界按 region size 校验。

## 11. 环境准备（guest 验证前）
### 11.0 编译xtools
> 编译host 侧，必做：本 guest 是 aarch64（`-M virt,iommu=smmuv3`），且 QEMU 用
> `-virtfs` 直接共享 host 的 `sysroot/linux/aarch64`。因此**每次改完代码都要用 aarch64 目标
> 重新编译**，否则 guest 里跑的还是旧 `xtools`（表现为日志内容/行号不更新）：
> ```sh
> ./devops.sh build --platform=linux --arch=aarch64
> ```
> - 产物：`sysroot/linux/aarch64/bin/xtools`（guest 经 9p 直接使用，**无需再拷贝**）；
> - **默认 `./devops.sh build --platform=linux` 是 x86_64**，不能用于本 guest；
> - ARM 构建会自动排除 `src/archive`、`object-stub` 等模块（见 [`mk/linux.cmake`](../mk/linux.cmake)）；
> - 改完可先自检：`strings -a sysroot/linux/aarch64/bin/xtools | grep "edu DMA done"`。

### 11.1 内核（[`linux-4.9.263/.config`](../linux-4.9.263/.config)）
已具备：`CONFIG_VFIO=y`、`CONFIG_VFIO_IOMMU_TYPE1=y`、`CONFIG_VFIO_PCI=y`。
**已启用**（本次完成）：`CONFIG_ARM_SMMU_V3=y`（原仅 `CONFIG_ARM_SMMU=y` SMMUv2）。
QEMU 的 `-M virt,iommu=smmuv3` 需要 guest 驱动 SMMUv3。启用方式（已执行）：
```sh
cp .config .config.vfio_backup            # 备份（已保留）
./scripts/config --enable ARM_SMMU_V3     # 命令行启用，不影响其它配置
make ARCH=arm64 olddefconfig              # 以现有 .config 为准补默认
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc) Image
# 验证：nm vmlinux | grep __of_table_arm_smmuv3   # 存在即已编入
# diff .config.vfio_backup .config                # 应仅多 CONFIG_ARM_SMMU_V3=y
```
> 原 `.config` 里 SPI/MTD/UIO/GPIO/VFIO/DEVMEM 等配置全部保留（已用 diff 验证）。

### 11.2 QEMU 启动（[`../qemu/build/qemu-system-aarch64`](../qemu/build/qemu-system-aarch64) 支持 smmuv3）
> **重要**：启用 `iommu=smmuv3` 时**不要**再用 `-dtb virt_custom.dtb` —— QEMU 的
> [`virt.c`](../qemu/hw/arm/virt.c:1607) 已把所有定制设备（`fpga`/`generic-uio`、i2c、
> spi0 alias、pl061 回环）写进自动生成设备树，且自动生成才包含 smmuv3 节点。
> 去掉 `-dtb` 让 QEMU 自动生成即可。

可直接使用的完整启动命令：
```sh
qemu-system-aarch64 \
  -M virt,iommu=smmuv3 \
  -cpu cortex-a57 \
  -m 2G \
  -kernel /home/alan/workspace/linux-4.9.263/arch/arm64/boot/Image \
  -initrd /home/alan/workspace/busybox-1.33.1/initramfs.cpio.gz \
  -nographic \
  -virtfs local,path=/home/alan/workspace/libobject/sysroot/linux/aarch64,mount_tag=host0,security_model=none,id=host0 \
  -device edu \
  -append "console=ttyAMA0 rdinit=/linuxrc uio_pdrv_genirq.compat_id=generic-uio uio_pdrv_genirq.of_id=generic-uio"
```
> 说明：
> - `-M virt,iommu=smmuv3`：启用 SMMUv3（[`virt.c`](../qemu/hw/arm/virt.c:2238) 用 iommu-map 绑到 pcie.0，edu 落 iommu_group）；
> - `-device edu`：edu（vendor 1234, device 11e8）；
> - `-virtfs .../sysroot/linux/aarch64`：9p 共享，guest 里挂载后即可用新的 xtools；
> - 不带 `-dtb`：自动生成设备树（含 smmuv3 + fpga + spi0 + pl061 回环）。

### 11.3 Guest 内完整测试命令序列
```sh
mkdir -p /mnt
mount -t 9p -o trans=virtio,version=9p2000.L host0 /mnt
export LD_LIBRARY_PATH=/mnt/lib
/mnt/bin/xtools --log-type=0 --log-level=0x16 mockery test_vfio_dma
```
> `Vfio_Pcie.open_device` 内部会尝试自动 `new_id`/`unbind`/`bind`（要求 root 且设备已在
> iommu_group，即 `-M virt,iommu=smmuv3`）。若自动绑定失败，按 C 步骤手动绑定一次。

### 11.4 预期日志（test_vfio_dma）
```sh
# 预期日志：
#   test_vfio_dma
#   edu device opened via VFIO, num_regions:..., num_irqs:...
#   edu id reg[0x00] = 0x10000ed
#   vfio dma_map success, buf:..., size:0x1000, iova:0x...
#   vfio register_irq ok, index:0, sub:0, ...
#   edu DMA done 2, src[0..3] = 00 01 02 03, dst[0..3] = 00 01 02 03
#   mem-to-mem DMA round-trip ok (guest mem -> edu dma_buf -> guest mem)
#   vfio dma demo ok (FROM_PCI + TO_PCI)
```
> 若 guest 未加 SMMUv3 / edu 未落 iommu_group，`open_device` 失败会优雅跳过，打印提示，不视为失败。

### 11.5 回归验证（#19）
```sh
xtools mockery test_gpio_event   # 依赖 pl061 line0→line1 回环（自动 DT 已含）
xtools mockery test_uio_pcie     # edu 寄存器路径（32 位取反修复验证）
```

## 12. 与 Uio_Pcie 的对比
| | Uio_Pcie（现有） | Vfio_Pcie（本实现） |
|---|---|---|
| BAR 映射 | sysfs 读物理地址 + mmap `/dev/mem` | 设备 fd 按 `vfio_region_info.offset` mmap（IOMMU 隔离） |
| 中断 | `/dev/uioX`（uio_pci_generic） | eventfd + io_worker |
| DMA | 不支持 | `VFIO_IOMMU_MAP_DMA`（共享内存/IOVA） |
| 隔离 | 无（/dev/mem） | IOMMU group 隔离 |
| 依赖 | UIO + CONFIG_DEVMEM | IOMMU（SMMU）+ vfio-pci |
| 寄存器访问 | `pcie_bar_addr` + read/write_register（偏移编码） | region_read/region_write（index=BAR/region、offset、width） |

## 13. VFIO 核心概念与理论（FAQ 整理）

> 本章整理 VFIO 使用中最容易混淆的概念：三层 fd、组/容器的共享与独占、
> eventfd 中断模型、`irq_index/sub_index`、向量、以及与 ARM GIC 中断的关系。

### 13.1 为什么打开 VFIO 需要三个 fd

VFIO 打开一个设备必须依次拿到 3 个 fd，对应"容器 → 组 → 设备"三层安全模型：

| # | 成员 | 来源 | 职责 |
|---|------|------|------|
| 1 | `container_fd` | `open("/dev/vfio/vfio")` | **容器**：一个 IOMMU 域，管理整个 IOVA 地址空间，`dma_map/dma_unmap` 都在它上面做 |
| 2 | `group_fd` | `open("/dev/vfio/<N>")` | **组**：IOMMU group（DMA 隔离最小单位），`VFIO_GROUP_SET_CONTAINER` 把它绑定到容器 |
| 3 | `device_fd` | `ioctl(group_fd, VFIO_GROUP_GET_DEVICE_FD, device_name)` | **设备**：真正的 PCI 设备句柄，region 查询/mmap/中断注册都在它上面做 |

为什么要三层而不是一个 fd，这是内核 VFIO 安全模型的硬性要求，核心是把
**"谁做 DMA"** 和 **"DMA 去往哪里"** 两个问题分开授权：

1. **安全隔离**：设备 DMA 必须经容器（IOMMU 域）申请 IOVA，由 IOMMU 页表隔离，
   设备永远无法直接访问未映射的物理内存（替代 UIO 直读 `/dev/mem`）。
2. **组是完整性保证**：IOMMU 无法隔离同组设备间的互访，必须**整个组**被 vfio-pci
   接管后才放行，group fd 即"组全占用"凭证。
3. **职责分离、可复用**：容器管 IOVA 空间（多组共享）、组管设备所有权、
   设备管寄存器/中断/DMA。
4. **生命周期逆序**：失败回滚时按"设备 → 组 → 容器"顺序 close
   （[`Vfio.c`](../src/board/hal/vfio/Vfio.c:168)）。

### 13.2 组（group）与容器（container）的共享 / 独占语义

两个概念要分清：

- **IOMMU group 是内核按 DMA 隔离拓扑算出的"系统属性"**，不是进程属性。
  组内设备彼此 DMA 不隔离，必须整体分配；组与组之间才隔离。
- **容器是进程创建的对象**（`open("/dev/vfio/vfio")`），默认私有，不代表全局共享。

独占规则（这是反复混淆的点）：

| 层面 | 是否独占 | 说明 |
|------|---------|------|
| 驱动绑定（bind vfio-pci） | 1:1 持续状态 | 一个设备同时只能绑一个驱动；是持久状态，不是每次使用绑一次 |
| `open("/dev/vfio/<N>")` | **不独占** | 所有进程都能 open 成功，只增引用计数 |
| `VFIO_GROUP_SET_CONTAINER` | **独占** | **一个组一次只能关联一个容器**，独立进程用自己的容器去 set 会失败 |
| `VFIO_GROUP_GET_DEVICE_FD` | 前提约束 | 组必须已关联容器且 viable，可多次拿多个设备 fd |

关键结论：

- 独占者是**容器**，不是 PID。共享同一容器 fd 的进程（fork 继承 / `SCM_RIGHTS` 传 fd）
  属于**同一个 owner**，可以并发访问同一设备；独立进程不行。
- 独占期 = "持有容器/组 fd 的期间"；全部 fd 关闭后组释放，可被重新占用。
- 这样设计是为了 **DMA 隔离**：同一时刻一个组只被一个 IOMMU 域管辖，
  避免两个进程各自建立冲突的 DMA 映射。

### 13.3 `/dev/vfio` 下只有组节点，没有设备节点

```
/dev/vfio/vfio    ← 容器节点（固定一个）
/dev/vfio/<N>     ← 组节点（每个 IOMMU group 一个）
```

- **没有** `/dev/vfio/<device>` 这类设备节点。
- 设备 fd 是运行时通过 `VFIO_GROUP_GET_DEVICE_FD` ioctl **动态取得的匿名 fd**
  （内核 `anon_inode_getfd`，无路径、可跨进程传 fd、随开随关）。
- 组节点 `/dev/vfio/<N>` 在**组内第一个设备绑定 vfio-pci** 时由内核创建；
  组内最后一个设备解绑后移除。
- 节点是否出现在文件系统取决于 devtmpfs/udev；无 udev 时由
  [`__ensure_group_devnode`](../src/board/hal/vfio/Vfio.c:50) 读 `/sys/class/vfio/<N>/dev`
  后 `mknod` 兜底。
- 与 **UIO** 的对比：UIO 是每设备一个静态节点 `/dev/uio0`、`/dev/uio1`，直接 `open`；
  VFIO 用"组节点 + 动态设备 fd"，让设备 fd 挂靠到容器/组的安全边界，更灵活安全。

### 13.4 中断为什么必须用 eventfd，而不是直接读 device_fd

- **`device_fd` 是 ioctl-only 的控制面**，VFIO 协议没有给它定义
  `read/poll` 来拿中断的语义。
- VFIO 中断交付模型（两步）：
  1. 用户创建 eventfd（[`Vfio.c:415`](../src/board/hal/vfio/Vfio.c:415)）；
  2. `ioctl(device_fd, VFIO_DEVICE_SET_IRQS)` 带
     `DATA_EVENTFD | ACTION_TRIGGER`，把第 `index` 组第 `sub_index` 号向量
     "路由"到该 eventfd（[`Vfio.c:420`](../src/board/hal/vfio/Vfio.c:420)）；
  3. 内核在设备中断时 `eventfd_signal()`，用户态 `poll/read` eventfd
     （本实现用 `io_worker` 监听 `EV_READ|EV_PERSIST`，[`Vfio.c:437`](../src/board/hal/vfio/Vfio.c:437)）。

为什么用 eventfd：

1. **通用通知原语**：可 `read/poll/epoll`、可非阻塞，能嵌入任意事件循环。
2. **多向量解复用**：每个 MSI-X 向量可路由到不同 eventfd，用户态能区分"哪个向量来的"，
   还能交给不同线程/进程处理。
3. **跨进程投递**：eventfd 可经 `SCM_RIGHTS` 传给别的进程（如 vhost）。
4. **计数防丢**：eventfd 是原子计数器，合并/电平触发不丢，`read` 原子清零
   （[`Vfio.c:361`](../src/board/hal/vfio/Vfio.c:361)）。

eventfd 的进程属性：它是**匿名 fd**，默认归创建进程，但可被 fork 继承或
`SCM_RIGHTS` 传递共享；内核在 `SET_IRQS` 时也会对 eventfd 文件本身持引用，
所以即使用户关掉 fd，中断仍会被写入（直到 `DATA_NONE` 清掉绑定）。

### 13.5 `irq_index` 与 `sub_index`

VFIO 用**二维参数**定位一个中断向量（对应 `struct vfio_irq_set` 的 `index`/`start`）：

| 参数 | 含义 | PCI 取值 |
|------|------|---------|
| `irq_index` | **中断组/类型**（选哪一类） | INTx=0、MSI=1、MSI-X=2（ERR=3、REQ=4） |
| `sub_index` | **组内向量号**（选第几个） | INTx 恒 0；MSI 0~31；MSI-X 0~2047 |

- 代码里 `set.index = irq_index`（[`Vfio.c:424`](../src/board/hal/vfio/Vfio.c:424)）、
  `set.start = sub_index`（[`Vfio.c:425`](../src/board/hal/vfio/Vfio.c:425)）、`set.count = 1`。
- `mask/unmask_irq` 语义相同（[`Vfio.c:469`](../src/board/hal/vfio/Vfio.c:469)）。

- **多向量支持**：每向量的注册状态统一收进 `vfio_irq_ctx_t`（efd/worker/handler/
  opaque/vfio/index/sub 合一），以 `irq_ctx[irq_index][向量]` 二维数组内嵌在 Vfio
  对象里（[`Vfio.h`](../src/include/libobject/board/hal/vfio/Vfio.h)，
  `VFIO_MAX_IRQ_VECTORS_PER_GROUP` 默认 64），无需动态分配；每个 (index, 向量) 可
  独立注册一个 eventfd + worker + handler。MSI-X 支持逐向量增量绑定（每次
  `SET_IRQS` 一个向量），不同 `sub_index` 注册互不影响。中断回调通过 worker->opaque
  指向的 `vfio_irq_ctx_t` 定位向量并分发到对应 handler；handler 内用
  `vfio_irq_get_vfio(opaque)` 取回 Vfio 对象。
### 13.6 VFIO 中断模型 vs ARM GIC 中断（SPI/PPI/SGI/LPI）

两个不同层次的概念：

- **ARM GIC**（硬件层，一维全局 ID）：SGI(0~15)、PPI(16~31)、SPI(32~1019)、
  LPI(8192+，GICv3 专供 MSI/MSI-X，需 ITS)。
- **VFIO `irq_index/sub_index`**（软件层，二维）：先选类型再选向量，与平台无关。

底层映射：

| VFIO 抽象 | x86 | ARM |
|-----------|-----|-----|
| index 0（INTx） | IO-APIC/本地 APIC | 一条 **SPI**（共享中断线） |
| index 1/2（MSI/MSI-X） | IRQ 重映射 → 本地 APIC | GICv3+ITS：`sub_index` 作为 EventID → **LPI** |

要点：

- **SPI 只对应 VFIO 里的 INTx 这一类**；MSI/MSI-X 在 ARM 上对应 LPI，不是 SPI。
- 用户态驱动只需认 VFIO 的 index/sub_index，**不需要知道**底层是 SPI 还是 LPI，
  换平台代码不用改。
- 直通场景：VFIO 管"物理侧把中断送到 eventfd"，QEMU 读到后向 **guest 的 vGIC**
  注入**虚拟**中断（guest SPI 号由 VMM 分配，与 host 物理号无固定对应）。

### 13.7 物理中断号（如 SPI 10）与 VFIO index 没有直接映射

- **`irq_index` 不是物理中断号**，而是"中断类型"。物理号在设备绑 vfio-pci 后被
  内核隐藏，用户态拿不到也不需要。
- **怎么确定用哪个 index**：按类型查 + 按优先级试：
  1. `VFIO_DEVICE_GET_INFO` 拿 `num_irqs`；
  2. 逐个 index 用 `VFIO_DEVICE_GET_IRQ_INFO` 查 `count`/`flags`（`count==0` 不可用）；
  3. 按 **MSI-X(2) → MSI(1) → INTx(0)** 优先级尝试 `SET_IRQS`，成功即用。
- **例外**：vfio-platform（DT 平台设备）的 `irq_index` = 设备树 `interrupts`
  属性的条目顺序（如第 0 条 SPI 10 → index 0）。
- 物理号只在调试/理解时看：`/proc/interrupts`、`/sys/bus/pci/devices/<bdf>/irq`、
  `/sys/.../msi_irqs/`。

### 13.8 向量（vector）是什么，为什么有这么多

**向量 = 设备可独立触发的一条编号化中断通道**（0,1,2,...），每条可单独使能/屏蔽/
路由到不同 CPU/绑定不同 handler。

- **INTx**：一根共享物理线，只有 1 个向量（sub_index 恒 0），无法切出多向量。
- **MSI/MSI-X**：**没有物理线**，靠"往内存写数据"触发中断。每个向量 = MSI-X 表
  里的一条表项（地址、数据、屏蔽位）；向量数 = **设备硬件实现的表条目数**
  （MSI ≤ 32，MSI-X ≤ 2048）。软件只是"启用几个 + 每个绑什么 handler"，
  **不是把 1 拆成 N**。

为什么需要这么多向量（性能/并行）：

1. **多队列并行**：网卡/NVMe 每队列一个向量，每个向量绑一个 CPU 核 → 多核无锁并行；
2. **区分事件**：完成/错误/管理各用独立向量，不用读状态寄存器；
3. **避免共享风暴**：INTx 共享会互相干扰，向量独立天然隔离；
4. **负载均衡**：每向量独立配置亲和性与中断合并。

**MSI-X 不是"虚拟中断"**：它和 INTx 一样是真实物理中断机制，只是"有线 vs 消息
（内存写）"的投递方式不同。真正叫"虚拟中断"的是虚拟化里 hypervisor 注入的
vIRQ、或 virtio 软件模拟的 MSI-X。

### 13.9 常见实现疑问

1. **`__get_group_path` 怎么知道用哪个 N**（多个 `/dev/vfio/<N>`）：
   不需要"选择"。每个设备只属于一个 iommu_group，代码 `readlink`
   `/sys/bus/pci/devices/<bdf>/iommu_group`，取链接目标路径**最后一段数字**作为 N
   （[`Vfio_Pcie.c:163`](../src/board/hal/vfio/Vfio_Pcie.c:163)）。N 由内核按
   IOMMU 拓扑预先分配，一个设备固定对应一个组。
2. **`__open_device` 多个 BDF 怎么选**：按 vendor/device 遍历
   `/sys/bus/pci/devices`，**取 `readdir` 扫到的第一个匹配**就 `break`
   （[`Vfio_Pcie.c:219`](../src/board/hal/vfio/Vfio_Pcie.c:219)）。
   `readdir` 顺序不是按 BDF 排序的，所以存在多块同型号设备时选择**不确定**；
   要精确选某块需给 `open_device` 加 BDF 入参或先排序再选。
3. **绑定 vfio-pci 后创建的是什么节点**：创建的是**组节点** `/dev/vfio/<N>`
   （一个组一个），**不是**某块 PCI 设备的专属节点；设备 fd 只能通过
   `VFIO_GROUP_GET_DEVICE_FD` 动态取。
4. **组号与"在 vfio 下"的区别**：iommu_group 编号由内核 IOMMU 子系统自动分配
   （设备位于 IOMMU 后就有，与驱动无关）；但 `/dev/vfio/<N>` 节点、组 viable
   需要设备**绑定 vfio-pci** 才会具备（由
   [`__bind_to_vfio`](../src/board/hal/vfio/Vfio_Pcie.c:96) 完成）。

### 13.10 IOVA 是"把 CPU 虚拟地址翻译过来"的吗？设备 DMA 寄存器认 IOVA 还是物理地址？

- **不是**。IOVA 是**独立的设备侧虚拟地址**，由驱动/内核自行挑选或分配，与 VA 无换算关系；
  VA 和 IOVA 是平级的两个虚拟地址，各自经 MMU / IOMMU 翻译到**同一块物理地址（PA）**。
- **设备 DMA 寄存器认的是"总线地址"**（Linux 里叫 `dma_addr_t`）：
  - **有 IOMMU 时**（本类场景，依赖 `CONFIG_VFIO_IOMMU_TYPE1`）→ 填 **IOVA**，由 IOMMU 翻成 PA；
  - **无 IOMMU / 直通时** → 填 **物理地址**，不经转换。
  两种情况驱动代码写法一致，因为 `dma_map` 已把"该填什么"抽象好了。
- **IOVA 是 dma_map 的输入**：ioctl 收到的是 `(vaddr, iova, size)`，只建 `IOVA→PA` 表项，
  不替你分配 IOVA（`*iova` 回传的就是传入值）；本实现用 `vfio->iova` 游标自行分配。
- 详细推导见 **6.4.1 地址模型：VA / PA / IOVA 三者关系**。

