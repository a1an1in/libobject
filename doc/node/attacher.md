attacher

## DLL Injection

动态库注入，英文名为Dynamic Link Library Injection，简称为DLL Injection

在Linux系统中，动态库注入的实现主要依赖于以下两种方法：

1. LD_PRELOAD环境变量方法：该方法是通过设置LD_PRELOAD环境变量来指定需要注入的动态库路径。在应用程序的执行过程中，该环境变量会强制指定需要注入的动态库，从而使得被注入的动态库中的函数和数据能够被应用程序调用和使用。

2. ptrace系统调用方法：该方法是通过调用ptrace系统调用来实现动态库注入。ptrace系统调用可以控制目标进程的执行，并允许开发人员读写进程的内存。通过ptrace系统调用可以将动态库注入到目标进程的地址空间中，从而实现动态库注入的目的。

## 计算函数地址
objdump -d bin|grep "func_name"

## refs

https://github.com/gaffe23/linux-inject/

[深入分析 LD_PRELOAD](https://blog.csdn.net/itworld123/article/details/125755603)

[二进制固件函数劫持术-DYNAMIC](https://zhuanlan.zhihu.com/p/524839954)

[Windows 中的三种常用 DLL 注入技术](https://www.xjx100.cn/news/475978.html?action=onClick)

[代码艺术：浅析GDB注入魔法](https://forum.butian.net/share/3070)