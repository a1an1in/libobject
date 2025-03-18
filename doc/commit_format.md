[update:attacher] 去掉attacher-builtin符号表。

Description:
在实际应用的过程中， 发现很多进程没有符号表， 所以去掉
attacher-builtin符号表， 以模拟真实场景。

Major Changes:
1. 尝试编译去掉部分库符号表。