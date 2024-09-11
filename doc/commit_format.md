[update:NA] 开发node。

Description:
修复bus_invoke_sync错误， 优化node日志。

Major Changes:
1. 修复bus bug， bus_handle_invoke_reply request 
   state赋值太早， 导致bus_invoke_sync有可能会段错误。
2. 优化node相关日志。