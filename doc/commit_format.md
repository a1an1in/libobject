[update:node] 解决mac平台node内存泄露问题。

Description:
销毁client或着server需要等待worker resign后销毁socket, 
目前只是简单等待，如果不等待，性能好的电脑有可能socket先退
出然后导致worker没有走退出流程导致内存泄露。

Major Changes:
1. 解决mac平台node内存泄露问题。