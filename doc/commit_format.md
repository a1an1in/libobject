[update:node] 解决mac平台node测试用例问题。

Description:
解决mac平台load问题， core和stub是动态库不能用force_load。

Major Changes:
1. 修复mac平台测试用例。
2. 修复mac平台bin链接动态库问题。