# Data Structure Document

[TOC]

## Test Iterfaces
* ./sysroot/linux/bin/xtools wget http://mirror.hust.edu.cn/gnu/hello/hello-1.3.tar.gz
* ./sysroot/linux/bin/xtools wget http://ftp.gnu.org/gnu/hello/hello-1.3.tar.gz
* ./sysroot/linux/bin/xtools wget http://mirrors.ustc.edu.cn/gnu/hello/hello-1.3.tar.gz
* ./sysroot/linux/bin/xtools player http://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8
* ./sysroot/linux/bin/xtools --event-thread-service=11111 --event-signal-service=11112 fshell
* ./sysroot/linux/bin/xtools --log-level=6 
* ./sysroot/linux/bin/xtools fshell --log-level=6
* ./sysroot/linux/bin/xtools --log-level=6  mockery test_skcipher_ecb_aes
* ./sysroot/linux/bin/xtools ctest -r=String_Test
* ./sysroot/windows/bin/xtools.exe mockery --log-level=6 --run-test=test_string
* ./sysroot/windows/bin/xtools.exe mockery --run-cmd=test_mockery_command --args=123,456
*     
*     

## timezone test
we can change system timezone using this command
```
timedatectl set-timezone Europe/Stockholm  # setting
timedatectl set-timezone Asia/Shanghai # setting
timedatectl                 # view current
timedatectl list-timezones  # list all
```