[update:doc] 编译完后安装头文件到sysroot。

Description:
兼容tangerine需要安装include， 更改./devops.sh后导致
包含libobject的库不能通过ExternalProject_Add编译， 
需要相应修改。

Major Changes:
1. 新增file-transfer目录.
2. 编译安装头文件