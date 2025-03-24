[update:node] 修复mset前空间没有清零的问题。

Description:
mset前变量未清零导致数据混淆。

Major Changes:
1. 修复mset问题。