# 使用方式
# 1. 执行 setup 命令
# attacher setup --log-level=0x20016 --filter="fd:12" --app-name="/bin/another_app"
# 2. 执行 call 命令
# attacher call --log-level=0x20016 "attacher_test_with_pointer_arg(0x1234, \"test2\")"
# 3. 执行 destroy 命令
# attacher destroy --log-level=0x20016
# 示例输出

# 全局变量
node_id=""

# 定义 node_cli 函数
node_cli() {
    # ~/.xtools/sysroot/bin/xtools node_cli --host="127.0.0.1" --service="12345" "$@"
    ./sysroot/linux/bin/xtools node_cli --host="127.0.0.1" --service="12345" "$@"
}

# setup 命令处理逻辑
attacher_setup() {
    # 设置环境变量
    export LD_LIBRARY_PATH=~/.xtools/sysroot/lib:~/.xtools/sysroot/lib64:$LD_LIBRARY_PATH
    ldconfig

    node_cli lookup all

    # 查找 node_id 和 pid
    node_id=$(node_cli lookup all | grep "$filter" | awk -F'id:' '{print $2}' | awk -F',' '{print $1}' | tr -d ' ')
    pid=$(ps aux | grep "$app_name" | grep -v grep | awk '{print $2}')

    # 输出调试信息
    echo "node_id: $node_id"
    echo "pid: $pid"

    if [[ -z "$node_id" || -z "$pid" ]]; then
        echo "Error: node_id or pid not found."
        return 1
    fi

    # 将 node_id 保存到文件
    echo "$node_id" > /tmp/attacher_node_id

    # 打开 attacher
    node_cli --log-level=$log_level call_bus $node_id@{"malloc(13, \"UnixAttacher\", #test_attacher, 0)"}
    node_cli --log-level=$log_level call_obj $node_id@{"attach(#test_attacher, $pid)"}
    node_cli --log-level=$log_level call_obj $node_id@{"init(#test_attacher)"}
    echo "Setup attacher for node_id:$node_id success!"
}

# call 命令处理逻辑
attacher_call() {
    # 从文件中读取 node_id
    if [[ -f /tmp/attacher_node_id ]]; then
        node_id=$(cat /tmp/attacher_node_id)
    else
        echo "Error: node_id is not set. Please run 'setup' first."
        return 1
    fi

    # 检查必要参数
    if [[ -z "$node_id" || -z "$func_str" ]]; then
        echo "node_id:$node_id func_str:$func_str error!"
        return 1
    fi

    # 分配内存
    node_cli --log-level=0x20014 call_bus $node_id@{"malloc(10, \"null\", #test_func_str, 128)"}
    node_cli --log-level=0x20014 call_bus $node_id@{"malloc(10, \"null\", #test_result, 128)"}

    # 设置函数字符串
    node_cli --log-level=0x20014 mset $node_id@#test_func_str{0-127} "$func_str"

    # 调用目标方法
    node_cli --log-level=$log_level call_obj $node_id@{"call(#test_attacher, 0, #test_func_str, #test_result)"}
    echo "Call attacher $node_id@$func_str success!"

    # 释放内存
    node_cli --log-level=0x20014 call_bus $node_id@{"mfree(#test_func_str)"}
    node_cli --log-level=0x20014 call_bus $node_id@{"mfree(#test_result)"}
}

# destroy 命令处理逻辑
attacher_destroy() {
    # 从文件中读取 node_id
    if [[ -f /tmp/attacher_node_id ]]; then
        node_id=$(cat /tmp/attacher_node_id)
    else
        echo "Error: node_id is not set. Please run 'setup' first."
        return 1
    fi

    # 检查 node_id 是否为空
    if [[ -z "$node_id" ]]; then
        echo "Error: node_id is empty. Please run 'setup' first."
        return 1
    fi

    # 销毁 attacher
    node_cli --log-level=$log_level call_bus $node_id@{"mfree(#test_attacher)"}
    echo "Destroy attacher for node_id:$node_id success!"

    # 删除保存的 node_id 文件
    rm -f /tmp/attacher_node_id
}

# 主函数
attacher() {
    local command=""
    local log_level="0x20014" # 默认日志等级
    local filter="fd:10"      # 默认过滤条件
    local app_name="/bin/plf_shared_app" # 默认进程名称
    local func_str=""

    # 解析参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            setup|call|destroy)
                command="$1" # 设置命令类型
                shift
                ;;
            --log-level=*)
                log_level="${1#*=}" # 提取日志等级
                shift
                ;;
            --filter=*)
                filter="${1#*=}" # 提取过滤条件
                shift
                ;;
            --app-name=*)
                app_name="${1#*=}" # 提取进程名称
                shift
                ;;
            *)
                if [[ "$command" == "call" ]]; then
                    if [[ -z "$func_str" ]]; then
                        func_str="$1" # 第一个非选项参数作为 func_str
                    fi
                fi
                shift
                ;;
        esac
    done

    # 检查命令类型
    if [[ -z "$command" ]]; then
        echo "Usage: attacher <setup|call|destroy> [options]"
        return 1
    fi

    # 根据命令类型调用对应的函数
    case $command in
        setup)
            attacher_setup
            ;;
        call)
            attacher_call
            ;;
        destroy)
            attacher_destroy
            ;;
        *)
            echo "Unknown command: $command"
            return 1
            ;;
    esac
}

attacher "$@"