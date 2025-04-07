[update:cmake] 修改windows cmake以适配本地和linux交叉编译。

Description:
linux支持交叉编译windows，这样编译服务器就可以很方便编译
windows平台， 不需要重复搭建编译环境。

Major Changes:
1. 修改windows本地cmake， 并增加本地工具链。
1. 新增inux交叉编译cmake。