[update:node] 修复node service source fd issue。

Description:
node serice src fd没有使用task里面保持的值，导致service
回复busd的src fd错误。

Major Changes:
1. 修改source fd问题。
2. 修改short task node_wait_flag置零。