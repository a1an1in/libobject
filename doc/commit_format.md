[update:Node] 实现node。

Description:
node持续实现过程中, 优化node_cli使用的打桩函数。

Major Changes:
1. 让fsh_add_stub_hooks支持函数名打桩， 因为node_cli
   不知道函数地址的， 专门提过接口去获取会比较麻烦。
2. fshell函数增加0dxxx来表示十进制数。