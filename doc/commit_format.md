[update:NA] 开发插件。

Description:
联合paddlefish plugin调试， 优化代码。

Major Changes:
1. 段错误直接退出，还没找到清楚段错误信号的方法， 会一直循环运行。
2. 优化Orm代码。
3. 把废弃的mysql和orm测试代码转为mockery。