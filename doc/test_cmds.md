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
addr2line -e ./sysroot/linux/bin/xtools 0x6a44f
addr2line -e ./sysroot/linux/bin/xtools 0x98c00
addr2line -e ./sysroot/linux/bin/xtools 0x53ccc
addr2line -f -e ./sysroot/linux/bin/xtools  libobject-core.so 0x43d6b
addr2line -e ./sysroot/linux/bin/xtools 0x6e96a
addr2line -e ./sysroot/linux/bin/xtools 0xaeba2
addr2line -e ./sysroot/linux/bin/xtools 0xb375b
addr2line -f -e ./sysroot/linux/bin/xtools  libobject-core.so +0x439a3
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
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 call_bus "node@set_loglevel(1,2,3)"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 call_bus "node@exit()"
./sysroot/linux/bin/xtools node_cli --log-level=0x20016 --host=127.0.0.1 --service=12345 fcopy ./tests/node/test_node.txt "node@./tests/node/output/write/test_node.txt"
./sysroot/linux/bin/xtools node_cli --log-level=0x20019 --host=127.0.0.1 --service=12345 fcopy ./tests/node/test_node.c "node@./tests/node/output/write/test_node.c"
./sysroot/linux/bin/xtools node_cli --log-level=0x20016 --host=127.0.0.1 --service=12345 fcopy ./tests/node/ "node@./tests/node/output/write/"
./sysroot/linux/bin/xtools --log-level=0x20016 node_cli --host=127.0.0.1 --service=12345 flist node@./tests/node/

./sysroot/windows/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --log-level=0x15 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/windows/bin/xtools node_cli --log-level=0x16 --host=127.0.0.1 --service=12345 flist 8d12cab570dcac3bf9d082e8141ae845612224a2@./tests/node/
./sysroot/windows/bin/xtools node_cli --log-level=0x16 --host=127.0.0.1 --service=12345 fcopy ./tests/node/test_node.c 8d12cab570dcac3bf9d082e8141ae845612224a2@./tests/node/output/write/test_node.c
./sysroot/windows/bin/xtools node_cli --log-level=0x16 --host=127.0.0.1 --service=12345 fcopy 8d12cab570dcac3bf9d082e8141ae845612224a2@./tests/node/res/test_node.txt ./tests/node/output/read/test_node.txt

/* test stub */
./sysroot/windows/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --log-level=0x15 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_bus "node@malloc(12, \"null\", #test_stub_name1, 0)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_bus "node@malloc(10, \"null\", #test_v1, 8)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_fsh "node@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_fsh "node@fsh_add_stub_hooks(#test_stub_name1, \"test_func\", \"test_print_inbound\", \"test_target_func\", \"test_print_outbound\", 7)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_fsh "node@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_fsh "node@fsh_remove_stub_hooks(#test_stub_name1)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_fsh "node@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_bus "node@mfree(#test_stub_name1)"
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_bus "node@mfree(#test_v1)"

./sysroot/windows/bin/xtools.exe mockery --log-level=0x14 -f test_node
./sysroot/linux/bin/xtools mockery --log-level=0x14 -f test_node

./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 --log-level=0x17 node --log-level=0x20016 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_cmd acdcda6a235bee5993530f7992d87330ac864a0d@{"ls -l"}
./sysroot/linux/bin/xtools node_cli --host=127.0.0.1 --service=12345 lookup all

object_id:acdcda6a235bee5993530f7992d87330ac864a0d
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
./devops.sh deploy -p=linux --host=139.159.231.27 --package-path=./packages/xtools_linux_v2.13.2.485.tar.gz

2 linux 测试
* 2.1 node cli 基本用法
nohup stdbuf -oL -eL ~/.xtools/sysroot/bin/xtools node --log-level=0x30016 --host=0.0.0.0 --service=12345 --deamon=t >~/.xtools/logs 2>&1 &
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x28017 flist b35f958b26e359bffe5c097e8c64150ec452b639@/root/.xtools/packages
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x28017 flist b35f958b26e359bffe5c097e8c64150ec452b639@./tests/node/
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20017 fcopy ./tests/node/res/test_node.txt "b35f958b26e359bffe5c097e8c64150ec452b639@./tests/node/output/write/test_node.txt"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_bus b35f958b26e359bffe5c097e8c64150ec452b639@{"set_loglevel(1,2,3)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_fsh b35f958b26e359bffe5c097e8c64150ec452b639@{"fsh_hello()"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_fsh b35f958b26e359bffe5c097e8c64150ec452b639@{"fsh_node_test()"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_cmd b35f958b26e359bffe5c097e8c64150ec452b639@{"ls -l"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_cmd b35f958b26e359bffe5c097e8c64150ec452b639@{"pwd"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 lookup all

* 2.2 stub
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_bus "b35f958b26e359bffe5c097e8c64150ec452b639@malloc(12, \"null\", #test_stub_name1, 0)"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_bus "b35f958b26e359bffe5c097e8c64150ec452b639@malloc(10, \"null\", #test_v1, 8)"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_fsh "b35f958b26e359bffe5c097e8c64150ec452b639@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_fsh "b35f958b26e359bffe5c097e8c64150ec452b639@fsh_add_stub_hooks(#test_stub_name1, \"test_func\", \"test_print_inbound\", \"test_target_func\", \"test_print_outbound\", 7)"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_fsh "b35f958b26e359bffe5c097e8c64150ec452b639@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_fsh "b35f958b26e359bffe5c097e8c64150ec452b639@fsh_remove_stub_hooks(#test_stub_name1)"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_fsh "b35f958b26e359bffe5c097e8c64150ec452b639@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_bus "b35f958b26e359bffe5c097e8c64150ec452b639@mfree(#test_stub_name1)"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_bus "b35f958b26e359bffe5c097e8c64150ec452b639@mfree(#test_v1)"

* 2.3 内存操作
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_bus b35f958b26e359bffe5c097e8c64150ec452b639@{"malloc(10, \"null\", #node_config, 128)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 mset b35f958b26e359bffe5c097e8c64150ec452b639@#node_config{0-127} "{\"log-level\": \"0x30016\",\"host\": \"139.159.231.27\",\"service\": \"12345\"}"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 mget b35f958b26e359bffe5c097e8c64150ec452b639@#node_config /100s
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 mget b35f958b26e359bffe5c097e8c64150ec452b639@#node_config /25xw
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 call_bus b35f958b26e359bffe5c097e8c64150ec452b639@{"mfree(#node_config)"}

* 2.4 迁移noded
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_bus b35f958b26e359bffe5c097e8c64150ec452b639@{"malloc(10, \"null\", #node_command, 8)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_fsh b35f958b26e359bffe5c097e8c64150ec452b639@{"node_command_get_global_addr(#node_command)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_bus b35f958b26e359bffe5c097e8c64150ec452b639@{"malloc(10, \"null\", #node_config, 128)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 mset b35f958b26e359bffe5c097e8c64150ec452b639@#node_config{0-127} "{\"log-level\": \"0x30015\"}"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_fsh b35f958b26e359bffe5c097e8c64150ec452b639@{"node_command_config(*#node_command, #node_config)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_bus b35f958b26e359bffe5c097e8c64150ec452b639@{"mfree(#node_config)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_bus b35f958b26e359bffe5c097e8c64150ec452b639@{"mfree(#node_command)"}

* 2.5 升级node
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_bus b35f958b26e359bffe5c097e8c64150ec452b639@{"malloc(10, \"null\", #node_command, 8)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_fsh b35f958b26e359bffe5c097e8c64150ec452b639@{"node_command_get_global_addr(#node_command)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_fsh b35f958b26e359bffe5c097e8c64150ec452b639@{"node_command_upgrade(*#node_command)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20016 call_bus b35f958b26e359bffe5c097e8c64150ec452b639@{"mfree(#node_command)"}

* 2.6 long task
stdbuf -oL -eL ./sysroot/linux/bin/test-process  > ~/.xtools/test_process.log 2>&1
./sysroot/linux/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --log-level=0x20016 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 node --log-level=0x20016 --host=127.0.0.1 --service=12345
./sysroot/linux/bin/xtools node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_cmd b35f958b26e359bffe5c097e8c64150ec452b639@{"tail -f ~/.xtools/test_process.log"}
./sysroot/linux/bin/xtools --event-thread-service=11151 --event-signal-service=11152 node_cli --log-level=0x14 --host=127.0.0.1 --service=12345 call_cmd b35f958b26e359bffe5c097e8c64150ec452b639@{"ls -l"}

stdbuf -oL -eL ./sysroot/linux/bin/test-process  > ~/.xtools/test_process.log 2>&1
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 node --log-level=0x20016 --host=139.159.231.27 --service=12345
./sysroot/linux/bin/xtools node_cli --log-level=0x14 --host=139.159.231.27 --service=12345 call_cmd b35f958b26e359bffe5c097e8c64150ec452b639@{"tail -f ~/.xtools/test_process.log"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 lookup all
./sysroot/linux/bin/xtools --event-thread-service=11151 --event-signal-service=11152 node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_cmd b35f958b26e359bffe5c097e8c64150ec452b639@{"ls -l"}

* 2.7 attancher
./sysroot/linux/bin/xtools mockery --log-level=0x14 -f test_attacher
./sysroot/linux/bin/xtools mockery --log-level=0x14 -f test_node

//run node service and test process
./sysroot/linux/bin/test-process
./sysroot/linux/bin/xtools --event-thread-service=11141 --event-signal-service=11142 node --log-level=0x20016 --host=139.159.231.27 --service=12345
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 lookup all

// open attacher
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_bus 4807d81c85887316271c05e27ee5a3f8795dd1a6@{"malloc(13, \"UnixAttacher\", #test_attacher, 0)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_obj 4807d81c85887316271c05e27ee5a3f8795dd1a6@{"attach(#test_attacher, 244049)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_obj 4807d81c85887316271c05e27ee5a3f8795dd1a6@{"init(#test_attacher)"}

// test adding stub
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_bus 4807d81c85887316271c05e27ee5a3f8795dd1a6@{"malloc(10, \"null\", #test_stub, 8)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_obj 4807d81c85887316271c05e27ee5a3f8795dd1a6@{"alloc_stub(#test_attacher, #test_stub)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_obj 4807d81c85887316271c05e27ee5a3f8795dd1a6@{"add_stub_hooks(#test_attacher, *#test_stub, \"test_with_mixed_type_pars\", \"attacher_test_with_pointer_arg_prehook\", \"attacher_test2_with_pointer_arg\", \"attacher_test_with_pointer_arg_posthook\", 2)"}

// remove stub
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_obj 4807d81c85887316271c05e27ee5a3f8795dd1a6@{"remove_stub_hooks(#test_attacher, *#test_stub)"}

// test calling target method
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_bus 4807d81c85887316271c05e27ee5a3f8795dd1a6@{"malloc(10, \"null\", #test_func_str, 128)"}
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 mset 4807d81c85887316271c05e27ee5a3f8795dd1a6@#test_func_str{0-127} "attacher_test_with_pointer_arg(0x1234, \"test2\")"
./sysroot/linux/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x20014 call_obj 4807d81c85887316271c05e27ee5a3f8795dd1a6@{"call(#test_attacher, 0, #test_func_str, 0)"}

3 windows 测试
./sysroot/windows/bin/xtools --event-thread-service=11131 --event-signal-service=11132 node --log-level=0x15 --host=139.159.231.27 --service=12345
./sysroot/windows/bin/xtools --log-level=0x20016 node_cli --host=139.159.231.27 --service=12345 flist 7917ec7c24809a2d718eeea06273d47f8fc9c3e7@/root/.xtools/packages
./sysroot/windows/bin/xtools --log-level=0x20016 node_cli --host=139.159.231.27 --service=12345 flist 9642a3c6dcc64bc451eba2a0da492e53178466f4@./tests/node/
./sysroot/windows/bin/xtools node_cli --host=139.159.231.27 --service=12345 --log-level=0x16 call_cmd e7fa912cd7d9f0ae6aeffdb40d78d714c10acbc8@{"ls -l"}
./sysroot/windows/bin/xtools node_cli --log-level=0x14 --host=139.159.231.27 --service=12345 lookup all
```