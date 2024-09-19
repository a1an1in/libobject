Daemontools是一个包含了很多管理Unix服务的工具的软件包。其中最核心的工具是supervise，它的功能是监控一个指定的服务，当该服务进程消亡，则重新启动该进程。
而要添加让supervise监控的服务非常容易，只需要添加一个被监控的服务的目录，在该目录中添加启动服务器的名字为run的脚本文件即可。


# refs
[supervise进程管理利器](https://blog.csdn.net/u012373815/article/details/70217030)