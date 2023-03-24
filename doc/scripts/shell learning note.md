# shell learning note

[TOC]

##  shell中${} 的用法
* 字符串的删除
    ```
    file=/dir1/dir2/dir3/my.file.txt
    ${file#*/}：删掉第一个/ 及其左边的字符串：dir1/dir2/dir3/my.file.txt
    ${file##*/}：删掉最后一个/  及其左边的字符串：my.file.txt
    ${file#*.}：删掉第一个.  及其左边的字符串：file.txt
    ${file##*.}：删掉最后一个.  及其左边的字符串：txt
    ${file%/*}：删掉最后一个 /  及其右边的字符串：/dir1/dir2/dir3
    ${file%%/*}：删掉第一个/  及其右边的字符串：(空值)
    ${file%.*}：删掉最后一个 .  及其右边的字符串：/dir1/dir2/dir3/my.file
    ${file%%.*}：删掉第一个 .   及其右边的字符串：/dir1/dir2/dir3/my
    记忆的方法为：
    # 是 去掉左边（键盘上#在 $ 的左边）
    %是去掉右边（键盘上% 在$ 的右边）
    单一符号是最小匹配；两个符号是最大匹配
    ```

* 字符串提取
    ```
    ${file:0:5}：提取最左边的5 个字节：/dir1
    ${file:5:5}：提取第5 个字节右边的连续5个字节：/dir2
    ```
* 字符串替换
    ```
    ${file/dir/path}：将第一个dir 替换为path：/path1/dir2/dir3/my.file.txt
    ${file//dir/path}：将全部dir 替换为path：/path1/path2/path3/my.file.txt
    ${#var} 可计算出变量值的长度：
    ${#file} 可得到27 ，因为/dir1/dir2/dir3/my.file.txt 是27个字节
    ```
* 字符串转换
    ${^^}将值都转换成大写
    ${,,}将值都转换成小写
## IF
* 字符串判断
    ```   
    [ -z STRING ] 如果STRING的长度为零则返回为真，即空是真
    [ -n STRING ] 如果STRING的长度非零则返回为真，即非空是真
    ```
* 数值判断
    ```
    [ INT1 -eq INT2 ] INT1和INT2两数相等返回为真 ,=
    [ INT1 -ne INT2 ] INT1和INT2两数不等返回为真 ,<>
    [ INT1 -gt INT2 ] INT1大于INT2返回为真 ,>
    [ INT1 -ge INT2 ] INT1大于等于INT2返回为真,>=
    [ INT1 -lt INT2 ] INT1小于INT2返回为真 ,<
    [ INT1 -le INT2 ] INT1小于等于INT2返回为真,<=
    ```
* 逻辑判断
    ```
    [ ! EXPR ] 逻辑非，如果 EXPR 是false则返回为真。
    [ EXPR1 -a EXPR2 ] 逻辑与，如果 EXPR1 and EXPR2 全真则返回为真。
    [ EXPR1 -o EXPR2 ] 逻辑或，如果 EXPR1 或者 EXPR2 为真则返回为真。
    [ ] || [ ] 用OR来合并两个条件
    [ ] && [ ] 用AND来合并两个条件
    ```
## 特殊符号
* $( ) 与 ``
在bash中，$( )与（反引号）都是用来作命令替换的。
命令替换是用来重组命令行的，先完成引号里的命令行，然后将其结果替换出来，再重组成新的命令行

[root@localhost ~]# echo today is $(date "+%Y-%m-%d")
today is 2017-11-07
[root@localhost ~]# echo today is `date "+%Y-%m-%d"`
today is 2017-11-07

⚠️注意：$( )的弊端是，并不是所有的类unix系统都支持这种方式，但反引号是肯定支持的

* ${ }
作用是获取变量的结果。一般情况下，$var与${var}是没有区别的，但是用${ }会比较精确的界定变量名称的范围

* $(( ))
作用是进行整数运算。 在 $(( )) 中的变量名称,可于其前面加 $ 符号来替换,也可以不用

进制转换
$(( ))可以将其他进制转成十进制数显示出来。用法如下：echo $((N#xx))
其中，N为进制，xx为该进制下某个数值，命令执行后可以得到该进制数转成十进制后的值
[root@localhost ~]# echo $((2#110))
6
[root@localhost ~]# echo $((16#2a))
42
[root@localhost ~]# echo $((8#11))
