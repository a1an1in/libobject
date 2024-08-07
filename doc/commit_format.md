[update:NA] 开发插件。

Description:
持续更新插件实现, 修复插件内注册class会导致段错误问题。
因为在释放plugin class时前面已经把plugin释放了，所以
需要的单独的plugin class的注册和注销宏。

Major Changes:
1. 新增PLUGIN_REGISTER_CLASS，PLUGIN_DEREGISTER_CLASS。
2. 新增class_deamon_deregister_class。