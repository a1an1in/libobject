[update:Node] 优化node alloc。

Description:
node alloc返回地址，这样更符合使用习惯。

Major Changes:
1. bus支持uint64_t参数。
2. node alloc返回值有字符串改为地址。