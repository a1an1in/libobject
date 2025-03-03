[update:attacher] 更改dlopen load标志。

Description:
load标志更改为RTLD_NOW，这会强制在 dlopen 返回前
完成所有符号的解析和初始化操作，包括执行构造函数函数。

Major Changes:
1. 更改dlopen load标志为RTLD_NOW。