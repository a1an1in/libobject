[update:eventbase] 去掉Event_Base端口配置。

Description:
之前的Event_Base需要指定两个端口号， 如果一台设备上运行多个xtools进程，
需要提前配置号每个进程使用的端口，会很麻烦。现在不需要配置端口号了，使用
系统随机分配的端口，这个端口也只是内部使用。

Major Changes:
1. 去掉Event_Base端口配置，简化命令行参数。