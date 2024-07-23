[update:NA] 开发插件。

Description:
因为Application需要支持load插件的功能， 会使用load动态库的
特性， 但是fshell在node里面定义，所以应该把fshell定义再往上
层提。即使需要使用load的插件的功能应该都有node，但是测试http
单元用例就没有node，所以就测试不了http插件案例。
另外，切片编程也需要在Application里面打桩函数的第二个实现，
需要fshell根据函数名获取地址的特性。
所以fshell定义在Application是很合理的。

Major Changes:
1. Application添加map管理插件。
2. Application定义fshell.