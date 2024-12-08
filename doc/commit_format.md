[update:node] 将node execute命令改为call_cmd。

Description:
更改execute命令名称，与call_bus保持一致， call_cmd
很容易看出这个是执行远端命令。mset表示执行node_cli
本地函数就很自然了。

Major Changes:
1. node 命令重命名。