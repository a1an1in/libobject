[update:NA] 联合paddlefish plugin调试。

Description:
优化代码。

Major Changes:
1. 去掉Number类。
2. 优化bitmap。
3. 去掉ploicy文件， 将相应实现放到对应对象中。
4. Model增加bitmap，这样就知道哪些数据有更新。
5. fix Row一个定义函数类型错误，导致cache有重复释放问题。