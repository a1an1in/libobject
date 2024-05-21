[update:Node] 新增node alloc。

Description:
fshell 调用有时需要提前分配内存 所以新增node alloc接口。

Major Changes:
1. 更改str_hex_to_int为str_hex_to_integer。
2. 新增node alloc。