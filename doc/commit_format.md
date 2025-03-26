[update:node] 优化attacher操作脚本。

Description:
之前node_id存放在/tmp/attacher_node_id文件，这样就限制同时
只能登录一个attacher。
attacher call有可能有返回值，这个返回值应该存放在提前分配的
内存中，所以需要新增attacher alloc和free内存的接口。

Major Changes:
1. 增加内存alloc和free。
2. node_id去掉文件依赖。