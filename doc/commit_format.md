[update:Node] 实现node。

Description:
node持续实现过程中, 优化node_cli使用的打桩函数。

Major Changes:
1. 去掉open_fsh 和close_fsh， node使用同一个shell，
   没有必要每次打开，现在默认都是打开的。
2. 将fshell从bus移动到node。
3. 更改日志等级。
4. 将$变量标志更换成#。