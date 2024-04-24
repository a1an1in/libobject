[update:mac] 修复mac平台编译问题.

Description:.
代码在持续演进过程中慢慢转移到linux和windows平台开发，导致mac
不能编译了， 修复mac编译问题。

Major Changes:
1. 去掉core里面的测试代码，因为mockery是从core分离出来的， mockery
   是依赖core的， 如果core里面包含测试代码，就会引起循环依赖问题。
2. 将add_module_lists和add_compiling_options放到平台的make文件里面，
   这样每个平台都有独立的配置， 不用写很多cmake if分支。