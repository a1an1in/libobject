[update:Node] 实现node。

Description:
node持续实现过程中, 优化node_cli使用的打桩函数。

Major Changes:
1. 去掉open_fsh， 因为node_cli只能使用字符串， 不能接收地址，
   所以必须使用变量名来访问地址。
2. fsh_add_stub_hooks支持使用函数名来打桩函数。
3. mfree支持使用变量名来释放内存。