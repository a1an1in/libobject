[update:NA] 开发插件。

Description:
持续更新插件实现, 修改object，以前Init_Nfunc_Entry和Init_Vfunc_Entry
处于混用的状态。 现在约束Init_Nfunc_Entry为私有函数， 不能覆盖和继承，
Init_Vfunc_Entry为虚函数， 可以覆盖和继承。 

Major Changes:
1. 去掉ENTRY_TYPE_IFUNC_POINTER类型，ENTRY_TYPE_VFUNC_POINTER 才支持继承和覆盖。
2. 将很多不规范的使用的宏Init_Nfunc_Entry改为Init_Vfunc_Entry
3. load_plugin支持运行插件。