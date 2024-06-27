[update:core] 新增struct operation类。

Description:
现在对象处理struct结构体很不方便，需要传各种处理函数，
所以像优化一下只要指定struct类型，然后后面对象就能处理
struct。

Major Changes:
1. 新增fsh_variable_info。