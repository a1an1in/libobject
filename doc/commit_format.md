[update:node] 去掉target_type_t类型。

Description:
之前设计mem ops可以兼容attacher和node，现在发现node没必要指定类型。
attacher mem ops可以直接调用对象方法。

Major Changes:
1. 去掉target_type_t类型后适配修改。