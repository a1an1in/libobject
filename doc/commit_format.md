[update:tests] 将mockery func测试用例移动到tests目录.

Description:
更改mockery头文件后，有部分还是使用register头文件的测试
用例找不到了， 所以借用这个机会把大部分用例移动到tests目录.

Major Changes:
1. 移动测试用例位置。
2. 更改测试用例注册头文件声明。