# shell learning note

[TOC]

## 字符串的用法

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

## 数组

* 定义

  ```bash
  array=(a b c)
  ```
* 添加

  ```bash
  array[0]="111"
  array[2]="222"
  ```
* 删除

  ```bash
  uset array[0] #删除元素0
  ```
* 遍历

  ```bash
  for((i=0;i<${#array[@]};i++))
  do
     echo ${array[i]};
  done
  ```
* \$*和\$@的区别

  \${array[*]}会将数组元素视为一个整体，而\${array[@]}将所有数组元素视为独立的个体，推荐使用$ {array[@]}

## 关联数组

* 定义

  declare -A site
* 使用方法

  site["google"]="www.google.com"

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

## 变量默认值


| **表达式**        | **含义**                                                      |
| ------------------- | --------------------------------------------------------------- |
| `${var}`          | 变量`var`的值，与`$var`相同                                   |
| `${var-DEFAULT}`  | 如果`var`没有被声明，那么就以`$DEFAULT`作为其值               |
| `${var:-DEFAULT}` | 如果`var`没有被声明，或者其值为空，那么就以`$DEFAULT`作为其值 |
| `${var=DEFAULT}`  | 如果`var`没有被声明，那么就以`$DEFAULT`作为其值               |
| `${var:=DEFAULT}` | 如果`var`没有被声明，或者其值为空，那么就以`$DEFAULT`作为其值 |
| `${var+OTHER}`    | 如果`var`声明了，那么其值就是$OTHER，否则就为`null`字符串     |
| `${var:+OTHER}`   | 如果`var`被设置了，那么其值就是$OTHER，否则就为`null`字符串   |
| `${var?ERR_MSG}`  | 如果`var`没被声明，那么就打印`$ERR_MSG`                       |
| `${var:?ERR_MSG}` | 如果`var`没被设置，那么就打印`$ERR_MSG`                       |
| `${!varprefix*}`  | 匹配之前所有以`varprefix`开头进行声明的变量                   |
| `${!varprefix@}`  | 匹配之前所有以`varprefix`开头进行声明的变量                   |

## 参数

```bash
参数处理说明
$# 传递到脚本的参数个数
$* 以一个单字符串显示所有向脚本传递的参数。 如"$*"用「"」括起来的情况、以"$1 $2 … $n"的形式输出所有参数。
$$ 脚本运行的当前进程ID号
$! 后台运行的最后一个进程的ID号
$*与$@：
  相同点：都是引用所有参数。
  不同点：只有在双引号中体现出来。假设在脚本运行时写了三个参数 1、2、3，，则 " * " 等价于 "1 2 3"(传递了一个参数)，而         "@" 等价于 "1" "2" "3"(传递了三个参数)。
$- 显示Shell使用的当前选项，与set命令功能相同。
$? 显示最后命令的退出状态。0表示没有错误，其他任何值表明有错误。
```

## 特殊符号

* $( ) 与 ``

  都是用来作命令替换的。命令替换是用来重组[命令行](https://so.csdn.net/so/search?q=%E5%91%BD%E4%BB%A4%E8%A1%8C&spm=1001.2101.3001.7020)的，先完成引号里的命令行，然后将其结果替换出来，再重组成新的命令行
* ${ }

  作用是获取变量的结果。一般情况下，`$var`与`${var}`是没有区别的，但是用${ }会比较精确的界定变量名称的范围
* $(( ))

  作用是进行整数运算。
* $((N#xx))

  将N进制数转换成十进制
