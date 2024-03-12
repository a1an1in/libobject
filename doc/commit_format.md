[update:ctest] 删除ctest和基于它写的关于Number类运算测试用例.

Description:
删除ctest， 这个库已经没有用了， mockery完全可以替代且更好用。Number类也应该删除
但是很多地方在使用需要逐步删除，先删除其测试用例。

Major Changes:
1. 删除ctest库， 因为使用复杂，而且mockery完全可以替代.
2. 删除Number测试用例。