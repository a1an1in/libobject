[update:node] 修复noded 内存泄漏问题。

Description:
event base在noded释放前退出，导致链路上内存未释放。

Major Changes:
1. node增加quit中断信号。