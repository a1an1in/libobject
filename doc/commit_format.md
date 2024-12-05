[update:node] 增加fshell变量取址能力。

Description:
fshell分配的变量做指针都是二级指针，函数定义的参数
大都是一级指针， 所以指针取址对fshell函数很重要。

Major Changes:
1. 实现fshell指针取址。