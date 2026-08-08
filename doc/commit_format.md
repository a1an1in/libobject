[update:dockerfile] 支持编译noded docker image。  

Description:
生成noded docker image， 方便部署。

Major Changes:
1. 制作noded docker image.

[Update:board] 重新设计嵌入式开发的目录结构。

Description:
更改drivers为board， 这个库不光是driver，还包括基于driver
的组件和app，这个board lib是专门开发嵌入式使用的。

Major Changes:
1. 规范目录结构。