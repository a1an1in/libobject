[update:attacher] 优化attacher，只需加载builtin库。

Description:
老版本本地和target都必须加载同版本的库，这个才能算出远端
函数的地址。优化后只需加载builtin库。

Major Changes:
1. 优化获取远端函数地址函数。
2. 去掉call_from_lib，因为call函数可以完全不用知道函数
   在哪个库了。