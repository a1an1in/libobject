[update:attacher] 更改attacher目标为行输出缓冲模式。

Description:
测试发现目标进程日志打印不出来，发现有可能是stdout缓冲模式
不对，修改为行缓冲，遇到\n就打印日志。

Major Changes:
1. 更改标准输出的缓冲模式。