# Data Structure Document

[TOC]

## mockery
```
./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x20016 -f all
```

## log 
```
./sysroot/linux/x86_64/bin/xtools --log-level=0x6      open all business at 6 level
./sysroot/linux/x86_64/bin/xtools --log-level=0x20006  open bus log at 6 level,  4 bits are debug level, higher bits are bussiness num.
```

## Test Iterfaces
* ./sysroot/linux/x86_64/bin/xtools wget http://mirror.hust.edu.cn/gnu/hello/hello-1.3.tar.gz
* ./sysroot/linux/x86_64/bin/xtools wget http://ftp.gnu.org/gnu/hello/hello-1.3.tar.gz
* ./sysroot/linux/x86_64/bin/xtools wget http://mirrors.ustc.edu.cn/gnu/hello/hello-1.3.tar.gz
* ./sysroot/linux/x86_64/bin/xtools player http://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8
* ./sysroot/linux/x86_64/bin/xtools fshell
* ./sysroot/linux/x86_64/bin/xtools fshell --log-level=0x16
* ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 test_mockery_command 123 456
* ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 -f test_string
* ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 -f test_stub_add_hooks
* ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 -f all
* ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 -f test_datetime_for_each_month
* ./sysroot/linux/x86_64/bin/xtools mockery test_inet_tcp_client
* ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x6 test_attacher
* ./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x16 -f test_message_publisher
* 
* ./sysroot/windows/bin/xtools.exe mockery --log-level=0x16 -f test_string
* ./sysroot/windows/bin/xtools.exe mockery --log-level=0x16 -f test_message_publisher
* ./sysroot/windows/bin/xtools.exe mockery --log-level=0x16 -f test_net

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
 * 使用方法如下：addr2line -e ./sysroot/linux/x86_64/bin/xtools 0x253dc
 * 注意： 使用的是offset， 不是[]号里面的绝对地址
 **/
./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x6 test_coredump_signal
addr2line -e ./sysroot/linux/x86_64/bin/xtools 0x6a44f
addr2line -e ./sysroot/linux/x86_64/bin/xtools 0x98c00
addr2line -e ./sysroot/linux/x86_64/bin/xtools 0x53ccc
addr2line -f -e ./sysroot/linux/x86_64/bin/xtools  libobject-core.so 0x43d6b
addr2line -e ./sysroot/linux/x86_64/bin/xtools 0x6e96a
addr2line -e ./sysroot/linux/x86_64/bin/xtools 0xaeba2
addr2line -e ./sysroot/linux/x86_64/bin/xtools +0x98479
addr2line -f -e ./sysroot/linux/x86_64/bin/xtools  libobject-core.so +0x439a3
```

## bus test
```
./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x20016 test_bus_server
./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x20016 test_bus_client_invoke_sync
./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x20016 test_bus_client_lookup_sync

./sysroot/windows/bin/xtools.exe mockery --log-level=0x30017 test_bus_server
./sysroot/windows/bin/xtools.exe mockery --log-level=0x20016 test_bus_client_invoke_sync
./sysroot/windows/bin/xtools.exe mockery --log-level=0x20016 test_bus_client_lookup_sync

```

## net tools
```
ping6 2409:8c20:1833:1000::ad5:2cb5
./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x6 test_client_udp_v6_recv
./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x6 test_client_udp_v6_send
./sysroot/windows/bin/xtools.exe mockery --log-level=0x14 -f test_http
./sysroot/windows/bin/xtools wget http://mirrors.hust.edu.cn/gnu/hello/hello-1.3.tar.gz
```