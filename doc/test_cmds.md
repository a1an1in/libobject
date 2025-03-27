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
* ./sysroot/linux/bin/xtools fshell
* ./sysroot/linux/bin/xtools fshell --log-level=0x16
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 test_mockery_command 123 456
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 -f test_string
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 -f test_stub_add_hooks
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 -f all
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 -f test_datetime_for_each_month
* ./sysroot/linux/bin/xtools mockery test_inet_tcp_client
* ./sysroot/linux/bin/xtools mockery --log-level=0x6 test_attacher
* ./sysroot/linux/bin/xtools mockery --log-level=0x16 -f test_message_publisher
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
./sysroot/linux/bin/xtools mockery --log-level=0x20016 test_bus_server
./sysroot/linux/bin/xtools mockery --log-level=0x20016 test_bus_client_invoke_sync
./sysroot/linux/bin/xtools mockery --log-level=0x20016 test_bus_client_lookup_sync

./sysroot/windows/bin/xtools.exe mockery --log-level=0x30017 test_bus_server
./sysroot/windows/bin/xtools.exe mockery --log-level=0x20016 test_bus_client_invoke_sync
./sysroot/windows/bin/xtools.exe mockery --log-level=0x20016 test_bus_client_lookup_sync

```

## net tools
```
ping6 2409:8c20:1833:1000::ad5:2cb5
./sysroot/linux/bin/xtools mockery --log-level=0x6 test_client_udp_v6_recv
./sysroot/linux/bin/xtools mockery --log-level=0x6 test_client_udp_v6_send
./sysroot/windows/bin/xtools.exe mockery --log-level=0x14 -f test_http
./sysroot/windows/bin/xtools wget http://mirrors.hust.edu.cn/gnu/hello/hello-1.3.tar.gz
```

## node
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
./sysroot/linux/bin/xtools node --log-level=0x30016 --host=0.0.0.0 --service=12345 --deamon=t 
./sysroot/linux/bin/xtools node --log-level=0x20016 --host=127.0.0.1 --service=12345
./devops.sh deploy -p=linux --host=139.159.231.27 --package-path=./packages/xtools_linux_x86_64_v2.14.0.125.tar.gz

node_cli() {
    # 检测操作系统类型
    if [[ "$(uname -s)" == "Linux" ]]; then
        ND_CLI="./sysroot/linux/bin/xtools node_cli"
    elif [[ "$(uname -s)" == "MINGW"* || "$(uname -s)" == "CYGWIN"* || "$(uname -s)" == "MSYS"* ]]; then
        ND_CLI="./sysroot/windows/bin/xtools.exe node_cli"
    else
        echo "Unsupported OS: $(uname -s)"
        return 1
    fi

    # 执行命令
    $ND_CLI --host="127.0.0.1" --service="12345" "$@"
}
node_id=$(node_cli lookup all | grep "index:2" | awk -F'id:' '{print $2}' | awk -F',' '{print $1}' | tr -d ' ')
node_cli lookup all

2 linux 测试
* 2.1 node cli 基本用法
nohup stdbuf -oL -eL ~/.xtools/sysroot/bin/xtools node --log-level=0x30016 --host=0.0.0.0 --service=12345 --deamon=t >~/.xtools/logs 2>&1 &
./sysroot/linux/bin/xtools node

node_cli --log-level=0x28017 flist $node_id@/root/.xtools/packages
node_cli --log-level=0x28017 flist $node_id@./tests/node/
node_cli --log-level=0x20017 fcopy ./tests/node/res/test_node.txt "$node_id@./tests/node/output/write/test_node.txt"
node_cli --log-level=0x20016 call_bus $node_id@{"set_loglevel(1,2,3)"}
node_cli --log-level=0x20016 call_fsh $node_id@{"fsh_hello()"}
node_cli --log-level=0x20016 call_fsh $node_id@{"fsh_node_test()"}
node_cli --log-level=0x20014 call_cmd $node_id@{"ls -l"}
node_cli --log-level=0x20014 call_cmd $node_id@{"pwd"}

* 2.2 stub
node_cli call_bus "$node_id@malloc(12, \"null\", #test_stub_name1, 0)"
node_cli call_bus "$node_id@malloc(10, \"null\", #test_v1, 8)"
node_cli call_fsh "$node_id@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
node_cli call_fsh "$node_id@fsh_add_stub_hooks(#test_stub_name1, \"test_func\", \"test_print_inbound\", \"test_target_func\", \"test_print_outbound\", 7)"
node_cli call_fsh "$node_id@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
node_cli call_fsh "$node_id@fsh_remove_stub_hooks(#test_stub_name1)"
node_cli call_fsh "$node_id@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
node_cli call_bus "$node_id@mfree(#test_stub_name1)"
node_cli call_bus "$node_id@mfree(#test_v1)"

* 2.3 内存操作
node_cli call_bus $node_id@{"malloc(10, \"null\", #node_config, 128)"}
node_cli mset $node_id@#node_config{0-127} "{\"log-level\": \"0x30016\",\"host\": \"139.159.231.27\",\"service\": \"12345\"}"
node_cli mget $node_id@#node_config /100s
node_cli mget $node_id@#node_config /25xw
node_cli call_bus $node_id@{"mfree(#node_config)"}

* 2.4 迁移noded
node_cli call_bus $node_id@{"malloc(10, \"null\", #node_command, 8)"}
node_cli call_fsh $node_id@{"node_command_get_global_addr(#node_command)"}
node_cli call_bus $node_id@{"malloc(10, \"null\", #node_config, 128)"}
node_cli mset $node_id@#node_config{0-127} "{\"log-level\": \"0x30015\"}"
node_cli call_fsh $node_id@{"node_command_config(*#node_command, #node_config)"}
node_cli call_bus $node_id@{"mfree(#node_config)"}
node_cli call_bus $node_id@{"mfree(#node_command)"}

* 2.5 升级node
node_cli call_bus $node_id@{"malloc(10, \"null\", #node_command, 8)"}
node_cli call_fsh $node_id@{"node_command_get_global_addr(#node_command)"}
node_cli call_fsh $node_id@{"node_command_upgrade(*#node_command)"}
node_cli call_bus $node_id@{"mfree(#node_command)"}

* 2.6 long task
stdbuf -oL -eL ./sysroot/linux/bin/test-process  > ~/.xtools/test_process.log 2>&1
./sysroot/linux/bin/xtools node --log-level=0x20016 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools node --log-level=0x20016 --host=127.0.0.1 --service=12345

node_cli call_cmd $node_id@{"tail -f ~/.xtools/test_process.log"}
node_cli call_cmd $node_id@{"ls -l"}

* 2.7 attancher
./sysroot/linux/bin/xtools mockery --log-level=0x14 -f test_attacher
./sysroot/linux/bin/xtools mockery --log-level=0x14 -f test_node

//run node service and test process
./sysroot/linux/bin/test-process
./sysroot/linux/bin/xtools node --log-level=0x20016 --host=127.0.0.1 --service=12345
pid=$(ps aux | grep test-process | grep -v grep | awk '{print $2}')

// open attacher
node_cli call_bus $node_id@{"malloc(13, \"UnixAttacher\", #test_attacher, 0)"}
node_cli call_obj $node_id@{"attach(#test_attacher, $pid)"}
node_cli call_obj $node_id@{"init(#test_attacher)"}

// test calling target method
node_cli call_bus $node_id@{"malloc(10, \"null\", #test_func_str, 128)"}
node_cli mset $node_id@#test_func_str{0-127} "attacher_test_with_pointer_arg(0x1234, \"test2\")"
node_cli call_obj $node_id@{"call(#test_attacher, 0, #test_func_str, 0)"}
node_cli call_bus $node_id@{"mfree(#test_attacher)"}

// test adding stub
node_cli call_bus $node_id@{"malloc(10, \"null\", #test_stub, 8)"}
node_cli call_obj $node_id@{"alloc_stub(#test_attacher, #test_stub)"}
node_cli call_obj $node_id@{"add_stub_hooks(#test_attacher, *#test_stub, \"test_with_mixed_type_pars\"， \"attacher_test_with_pointer_arg_prehook\", \"attacher_test2_with_pointer_arg\", \"attacher_test_with_pointer_arg_posthook\", 2)"}

// remove stub
node_cli call_obj $node_id@{"remove_stub_hooks(#test_attacher, *#test_stub)"}

* 2.8 脚本简化attacher操作
./sysroot/linux/bin/test-process
./sysroot/linux/bin/xtools node --log-level=0x30016 --host=127.0.0.1 --service=12345 --deamon=t
sudo ./sysroot/linux/bin/xtools node --log-level=0x20016 --host=127.0.0.1 --service=12345

source ./sysroot/linux/bin/attacher.sh
attacher setup --filter="fd:10" --app-name="test-process"
attacher alloc "#test_result"
attacher call --log-level=0x20014 "attacher_test_with_pointer_arg(0x1234, \"test2\")" "#test_result"
attacher free "#test_result"
attacher destroy

3 windows 测试
./sysroot/windows/bin/xtools node --log-level=0x15 --host=139.159.231.27 --service=12345
node_cli --log-level=0x20016 flist $node_id@/root/.xtools/packages
node_cli --log-level=0x20016 flist $node_id@./tests/node/
node_cli --log-level=0x20016 call_cmd $node_id@{"ls -l"}
node_cli --log-level=0x20016 lookup all
```