[update:core] 更改REGISTER_CLASS 宏参数。

Description:
老的REGISTER_CLASS定义方式限制了一个文件只能定义一次宏，
这次提交修改了这个约束。

Major Changes:
1. 修改REGISTER_CLASS宏定义。