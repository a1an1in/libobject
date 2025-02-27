[update:attacher] 新增一个重定向attacher的日志文件。

Description:
attacher调试时有时目标进程日志丢失输出很难调试，新增一个
重定向标准输出的功能。

Major Changes:
1. attacher init直接重定向标准输出。