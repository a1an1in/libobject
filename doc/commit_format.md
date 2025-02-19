[update:node] 解决mac平台node内存泄露问题。

Description:
销毁client或着server需要等待worker resign后销毁socket, 
目前只是简单等待，如果不等待，性能好的电脑有可能socket先退
出然后导致worker没有走退出流程导致内存泄露。

Major Changes:
1. 加载完libattacher-builtin.so后，动态库加载使用attacher_dlopen， 
   如果有错误这个可以打印错误原因。
2. 修复read_configs 局部变量为初始化问题。