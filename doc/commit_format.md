[update:dockerfile] 支持编译noded docker image。  

Description:
生成noded docker image， 方便部署。

Major Changes:
1. 制作noded docker image.

[Add:vfio] 新增Vfio_Pcie类，支持用户态安全驱动 PCIe 设备开发。

Description:
Vfio_Pcie 能用来做所有"用户态安全驱动 PCIe 设备"的事：高性能网卡/NVMe/GPU 
用户态驱动、FPGA 加速卡管理、QEMU 设备直通、教学与测试平台、以及没有内核驱动
的定制板卡。它把 VFIO 的公共骨架（发现/BAR/寄存器/DMA/中断）抽成基类，接新
设备只需写一个子类。

Major Changes:
1. 新增Vfio_Pcie类及其测试用例。