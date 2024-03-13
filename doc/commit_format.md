[update:concurrent] 移动message到concurrent.

Description:
因为message是基于concurrent的io_worker实现的。可以理解成concurrent
模块提供的一个简单的进程间通信机制。

Major Changes:
1. 移动message目录。