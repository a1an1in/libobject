[update:windows] 解决windows编译依赖问题。

Description:
第三方库新增regex, 然后把windows-dl变为子模块，不再
使用git托管， 以前的编译太复杂。这样windows编译就没有
依赖问题了。

Major Changes:
1. 新增regex到3rd目录。
2. 更改windows-dl变为cmake子模块。