[update:node] node已经可以在远端正常运行。

Description:
node在本地测试正常，但是把node部署到远端发现有问题，
根本原因是bus基于tcp协议，在远端运行大的数据有可能
会分片，但是bus以前设计没有考虑这个问题。
v2.13.1.1是可以基于bus进行点对点通信的第一个版本。
这个是一个比较有意义的节点。

Major Changes:
1. 解决bus和busd以支持tcp分片问题。
2. 解决node id长度不是固定20字节问题。
3. 更改work_task_s的request属性名为cache。
4. 更改worker负责分配work task。