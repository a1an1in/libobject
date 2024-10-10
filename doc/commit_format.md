[update:bus] 新增bus断链后恢复机制。

Description:
如果busd掉线后，所有的bus都需要重启才能恢复正常。
这样就会在实际使用的出现问题， 所以新增bus恢复机制。

Major Changes:
1. 新增定时器, 断链后恢复bus service。