[update:http] app不管理插件。

Description:
app管理所有的插件有问题， 在测试的时候发现在销毁插件是已经
找不到对应的应用了， 因为应用已经销毁了，所以需要把插件交给
应用管理， 比如httpd command。

Major Changes:
1. 更改http插件配置文件名。
2. 测试用例拷贝http插件配置文件到工作目录。
3. 插件从app更改到httpd command。