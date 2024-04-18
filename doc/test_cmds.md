# Data Structure Document

[TOC]

## mockery
```
./sysroot/linux/bin/xtools mockery --log-level=0x20016 -f all
```

## log 
```
./sysroot/linux/bin/xtools --log-level=0x6      open all business at 6 level
./sysroot/linux/bin/xtools --log-level=0x20006  open bus log at 6 level,  4 bits are debug level, higher bits are bussiness num.
```

## Test Iterfaces
* ./sysroot/linux/bin/xtools wget http://mirror.hust.edu.cn/gnu/hello/hello-1.3.tar.gz
* ./sysroot/linux/bin/xtools wget http://ftp.gnu.org/gnu/hello/hello-1.3.tar.gz
* ./sysroot/linux/bin/xtools wget http://mirrors.ustc.edu.cn/gnu/hello/hello-1.3.tar.gz
* ./sysroot/linux/bin/xtools player http://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8
* ./sysroot/linux/bin/xtools --event-thread-service=11111 --event-signal-service=11112 fshell
* ./sysroot/linux/bin/xtools fshell --log-level=0x16
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 test_mockery_command 123 456
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 -f test_string
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 -f test_stub_add_hooks
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 -f all
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 -f test_datetime_for_each_month
* ./sysroot/linux/bin/xtools --event-thread-service=11111 --event-signal-service=11121 mockery test_inet_tcp_client
* ./sysroot/linux/bin/xtools mockery --log-level=0x6 test_attacher
* sudo ./sysroot/linux/bin/xtools mockery --log-level=0x6 test_attacher_call_from_lib 94989
* 
* ./sysroot/windows/bin/xtools.exe mockery --log-level=0x16 -f test_string
* ./sysroot/windows/bin/xtools.exe mockery --log-level=0x16 -f test_message_publisher
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
./sysroot/linux/bin/xtools mockery --log-level=0x6 test_coredump_signal
addr2line -e ./sysroot/linux/bin/xtools 0x253dc
addr2line -e ./sysroot/linux/bin/xtools 0x98c00
addr2line -e ./sysroot/linux/bin/xtools 0x53ccc
addr2line -f -e ./sysroot/linux/bin/xtools  libobject-core.so 0x152
```

## bus test
```
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 mockery --log-level=0x20016 test_bus_server
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x20016 test_bus_client_invoke_sync
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x20016 test_bus_client_lookup_sync

./sysroot/windows/bin/xtools.exe --event-thread-service=11131 --event-signal-service=11132 mockery --log-level=0x30017 test_bus_server
./sysroot/windows/bin/xtools.exe --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x20016 test_bus_client_invoke_sync
./sysroot/windows/bin/xtools.exe --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x20016 test_bus_client_lookup_sync

```

## net
```
ping6 2409:8c20:1833:1000::ad5:2cb5
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 mockery --log-level=0x6 test_client_udp_v6_recv
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x6 test_client_udp_v6_send
```

## node
```
/* test exit */
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 --log-level=0x16 node --log-level=0x20016 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x20016 test_node_invoke_exit

/* test loglevel */
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x20016 test_node_invoke_setloglevel
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x20016 test_node_invoke_exit

/* test node cli */
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --log-level=0x20017 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 --call_bus="node@set_loglevel(1,2,3)"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 call_bus "node@set_loglevel(1,2,3)"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 call_bus "node@exit()"
./sysroot/linux/bin/xtools node_cli --log-level=0x20016 --host=127.0.0.1 --service=12345 copy ./tests/node/test_node.txt "node:./tests/node/output/write/test_node.txt"
./sysroot/linux/bin/xtools node_cli --log-level=0x20019 --host=127.0.0.1 --service=12345 copy ./tests/node/test_node.c "node:./tests/node/output/write/test_node.c"
./sysroot/linux/bin/xtools node_cli --log-level=0x20016 --host=127.0.0.1 --service=12345 copy ./tests/node/ "node:./tests/node/output/write/"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 call_bus "node@list(./tests/node/)"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 list node:./tests/node/

./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 call_bus "node@open_fshell()"
./sysroot/linux/bin/xtools node_cli --host=127.0.0.1 --service=12345 --log-level=0x20016 execute_fsh "node@fs_add(1,2)"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 call_bus "node@close_fshell()"
./sysroot/linux/bin/xtools mockery --log-level=0x16 test_read_file
./sysroot/linux/bin/xtools mockery --log-level=0x16 test_write_file
./sysroot/linux/bin/xtools mockery --log-level=0x16 -f test_node

./sysroot/windows/bin/xtools.exe mockery --log-level=0x16 -f test_node

```