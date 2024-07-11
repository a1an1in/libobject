[update:node] 重构重构node_malloc和node_mfree。

Description:
重构node_malloc和node_mfree，复用Struct_Adapter的方法。

Major Changes:
1. 新增Struct_Adapter alloc方法.
2. 重构node_malloc和node_mfree，复用fsh_variable_info方法。
