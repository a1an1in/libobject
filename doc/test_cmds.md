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
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 -f test_message_publisher
* sudo ./sysroot/linux/bin/xtools mockery --log-level=0x6 test_attacher_call_from_lib 94989
* 
* ./sysroot/windows/bin/xtools.exe mockery --log-level=0x16 -f test_string
* ./sysroot/windows/bin/xtools.exe mockery --log-level=0x16 -f test_message_publisher
* ./sysroot/windows/bin/xtools.exe mockery --log-level=0x16 -f test_net
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
addr2line -e ./sysroot/linux/bin/xtools 0x93be6
addr2line -e ./sysroot/linux/bin/xtools 0xaeba2
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

## net tools
```
ping6 2409:8c20:1833:1000::ad5:2cb5
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 mockery --log-level=0x6 test_client_udp_v6_recv
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x6 test_client_udp_v6_send
./sysroot/windows/bin/xtools.exe mockery --log-level=0x14 -f test_http
./sysroot/windows/bin/xtools wget http://mirrors.hust.edu.cn/gnu/hello/hello-1.3.tar.gz
```

## node
### 本地测试
```
/* test exit */
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 --log-level=0x17 node --log-level=0x20016 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x20016 test_node_invoke_exit

/* test loglevel */
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x20016 test_node_invoke_setloglevel
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 mockery --log-level=0x20016 test_node_invoke_exit

/* test node cli */
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --log-level=0x15 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 --call_bus="node@set_loglevel(1,2,3)"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 call_bus "node@set_loglevel(1,2,3)"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 call_bus "node@exit()"
./sysroot/linux/bin/xtools node_cli --log-level=0x20016 --host=127.0.0.1 --service=12345 copy ./tests/node/test_node.txt "node@./tests/node/output/write/test_node.txt"
./sysroot/linux/bin/xtools node_cli --log-level=0x20019 --host=127.0.0.1 --service=12345 copy ./tests/node/test_node.c "node@./tests/node/output/write/test_node.c"
./sysroot/linux/bin/xtools node_cli --log-level=0x20016 --host=127.0.0.1 --service=12345 copy ./tests/node/ "node@./tests/node/output/write/"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 call_bus "node@list(./tests/node/)"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 list node@./tests/node/

./sysroot/windows/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --log-level=0x15 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/windows/bin/xtools node_cli --log-level=0x16 --host=127.0.0.1 --service=12345 list 8d12cab570dcac3bf9d082e8141ae845612224a2@./tests/node/
./sysroot/windows/bin/xtools node_cli --log-level=0x16 --host=127.0.0.1 --service=12345 copy ./tests/node/test_node.c 8d12cab570dcac3bf9d082e8141ae845612224a2@./tests/node/output/write/test_node.c
./sysroot/windows/bin/xtools node_cli --log-level=0x16 --host=127.0.0.1 --service=12345 copy 8d12cab570dcac3bf9d082e8141ae845612224a2@./tests/node/res/test_node.txt ./tests/node/output/read/test_node.txt

/* test stub */
./sysroot/windows/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --log-level=0x15 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_bus "node@malloc(0, 12, \"null\", #test_stub_name1, 0)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_bus "node@malloc(0, 10, \"null\", #test_v1, 8)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_fsh "node@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_fsh "node@fsh_add_stub_hooks(#test_stub_name1, \"test_func\", \"test_print_inbound\", \"test_target_func\", \"test_print_outbound\", 7)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_fsh "node@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_fsh "node@fsh_remove_stub_hooks(#test_stub_name1)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_fsh "node@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_bus "node@mfree(0, #test_stub_name1)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_bus "node@mfree(0, #test_v1)"

./sysroot/windows/bin/xtools.exe mockery --log-level=0x14 -f test_node
./sysroot/linux/bin/xtools mockery --log-level=0x20017 -f test_node

./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 --log-level=0x17 node --log-level=0x20016 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools  node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 execute 1b0fd2d7e9f2249372b350c8fe0e9732fc02a996@{"ls -l"}
object_id:1b0fd2d7e9f2249372b350c8fe0e9732fc02a996
```

### 线上测试
```
./devops.sh build --platform=windows
./devops.sh docker --install --platform=linux    #install docker at linux platform

export LD_LIBRARY_PATH=~/.xtools/sysroot/lib:~/.xtools/sysroot/lib64:$LD_LIBRARY_PATH
ldconfig

1 部署
tcpdump -i eth0  port 12345
sudo tcpdump -i enp0s17  port 12345
./devops.sh build --platform=linux
./devops.sh release -p=linux
./devops.sh deploy -p=linux --host=139.159.231.27 --package-path=./packages/xtools_linux_v2.13.2.216.tar.gz

2 linux 测试
nohup stdbuf -oL -eL ~/.xtools/sysroot/bin/xtools node --log-level=0x30016 --host=0.0.0.0 --service=12345 --deamon=t >~/.xtools/logs 2>&1 &
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --log-level=0x30016 --host=139.159.231.27 --service=12345
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x28017 list 8dbfb5f2e674dddcb82acc2e67eba709c9479761@/root/.xtools/packages
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x28017 list 8c7175239264424728f8ce9cafcea56a609233a9@./tests/node/
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_bus ec52cbeaa14dd3898941f21a923924c59d7a4b4a@{"set_loglevel(1,2,3)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_fsh ec52cbeaa14dd3898941f21a923924c59d7a4b4a@{"test_hello()"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 execute 8ffcb3d302a1c06d0c4b9e0d5063aaab2ea0e49c@{"ls -l"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 execute 8ffcb3d302a1c06d0c4b9e0d5063aaab2ea0e49c@{"pwd"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 lookup all

3 windows 测试
./sysroot/windows/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --log-level=0x15 --host=139.159.231.27 --service=12345
./sysroot/windows/bin/xtools --log-level=0x20016 node_cli --host=139.159.231.27 --service=12345 list 7917ec7c24809a2d718eeea06273d47f8fc9c3e7@/root/.xtools/packages
./sysroot/windows/bin/xtools --log-level=0x20016 node_cli --host=139.159.231.27 --service=12345 list 9642a3c6dcc64bc451eba2a0da492e53178466f4@./tests/node/
./sysroot/windows/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x16 execute e7fa912cd7d9f0ae6aeffdb40d78d714c10acbc8@{"ls -l"}
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=139.159.231.27 --service=12345 lookup all
id:e7fa912cd7d9f0ae6aeffdb40d78d714c10acbc8

```