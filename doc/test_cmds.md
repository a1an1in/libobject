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
* ./sysroot/linux/bin/xtools ctest -r=Hash_Map_Test
* ./sysroot/linux/bin/xtools mockery --log-level=6 test_mockery_command 123 456
* ./sysroot/linux/bin/xtools mockery --log-level=6 test_string
* ./sysroot/linux/bin/xtools mockery --log-level=6 test_stub_add_hooks
* ./sysroot/linux/bin/xtools mockery --log-level=6 all
* ./sysroot/linux/bin/xtools mockery test_datetime_for_each_month
* ./sysroot/linux/bin/xtools --event-thread-service=11111 --event-signal-service=11121 mockery test_inet_tcp_client
* ./sysroot/linux/bin/xtools mockery --log-level=6 test_attacher
* sudo ./sysroot/linux/bin/xtools mockery --log-level=6 test_attacher_call_from_lib 94989
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

## coredump test
```
/* backtrace 可以打印出调用栈地址， 然后使用addr2line可以把地址转换从文件对应行
 * 使用方法如下：addr2line -e ./sysroot/linux/bin/xtools 0x253dc
 * 注意： 使用的是offset， 不是[]号里面的绝对地址
 **/
./sysroot/linux/bin/xtools mockery --log-level=6 test_coredump_signal
addr2line -e ./sysroot/linux/bin/xtools 0x253dc
addr2line -e ./sysroot/linux/bin/xtools 0x98c00
```

## bus test
```
./sysroot/linux/bin/xtools --event-thread-service=11121 --event-signal-service=11122 mockery --log-level=6 test_bus_daemon
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 mockery --log-level=6 test_bus_server
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=6 test_bus_client

```