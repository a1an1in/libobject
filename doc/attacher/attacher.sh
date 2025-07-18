# 使用方式:
# 1. 执行 setup 命令
#    初始化 attacher，设置环境并获取 node_id 和 pid。
#    示例:
#    attacher setup --log-level=0x20016 --filter="fd:12" --app-name="/bin/another_app"
#
# 2. 执行 call 命令
#    调用目标方法，可以选择传递 output_var。
#    示例:
#    - 不传递 output_var:
#      attacher call --log-level=0x20016 "attacher_test_with_pointer_arg(0x1234, \"test2\")"
#    - 传递自定义 output_var:
#      attacher call --log-level=0x20016 "attacher_test_with_pointer_arg(0x1234, \"test2\")" "#custom_output"
#
# 3. 执行 alloc 命令
#    分配 output_var 的内存。
#    示例:
#    attacher alloc --log-level=0x20014 "#custom_output"
#
# 4. 执行 free 命令
#    释放 output_var 的内存。
#    示例:
#    attacher free --log-level=0x20014 "#custom_output"
#
# 5. 执行 destroy 命令
#    销毁 attacher，释放相关资源。
#    示例:
#    attacher destroy --log-level=0x20016
#
# 注意事项:
# - setup 命令会初始化 node_id 和 pid，必须在其他命令之前运行。
# - output_var 的内存管理需要通过 alloc 和 free 命令显式处理。
# - 如果未正确释放内存，可能会导致资源泄漏。
# - 使用 source 加载脚本后，所有命令都可以直接调用。

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

    # 打开 attacher
    node_cli --log-level=$log_level call_bus $node_id@{"malloc(13, \"UnixAttacher\", #test_attacher, 0)"}
    node_cli --log-level=$log_level call_obj $node_id@{"attach(#test_attacher, $pid)"}
    node_cli --log-level=$log_level call_obj $node_id@{"init(#test_attacher)"}
    echo "Setup attacher for node_id:$node_id success!"
}

# call 命令处理逻辑
attacher_call() {
    # 检查 node_id 是否已设置
    if [[ -z "$node_id" ]]; then
        echo "Error: node_id is not set. Please run 'setup' first."
        return 1
    fi

    # 检查必要参数
    if [[ -z "$func_str" ]]; then
        echo "Error: func_str is required for 'call' command."
        return 1
    fi

    # 如果未传入 output_var，则默认为 0
    local output_var="${1:-0}"

    # 分配内存给 func_str
    node_cli --log-level=0x20014 call_bus $node_id@{"malloc(10, \"null\", #test_func_str, 128)"}

    # 设置函数字符串
    node_cli --log-level=0x20014 mset $node_id@#test_func_str{0-127} "$func_str"

    # 调用目标方法
    node_cli --log-level=$log_level call_obj $node_id@{"call(#test_attacher, 0, #test_func_str, $output_var)"}

    echo "Call attacher $node_id@$func_str success!"

    # 释放 func_str 的内存
    node_cli --log-level=0x20014 call_bus $node_id@{"mfree(#test_func_str)"}
}

# destroy 命令处理逻辑
attacher_destroy() {
    # 检查 node_id 是否已设置
    if [[ -z "$node_id" ]]; then
        echo "Error: node_id is not set. Please run 'setup' first."
        return 1
    fi

    # 销毁 attacher
    node_cli --log-level=$log_level call_bus $node_id@{"mfree(#test_attacher)"}
    echo "Destroy attacher for node_id:$node_id success!"

    # 清空全局变量 node_id
    node_id=""
}

# 主函数
attacher() {
    local command=""
    local log_level="0x20014" # 默认日志等级
    local filter="fd:10"      # 默认过滤条件
    local app_name="/bin/plf_shared_app" # 默认进程名称
    local func_str=""
    local output_var="0" # 默认输出变量为 0

    # 解析参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            setup|call|destroy|alloc|free)
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
                    elif [[ "$output_var" == "0" ]]; then
                        output_var="$1" # 第二个非选项参数作为 output_var
                    fi
                elif [[ "$command" == "alloc" || "$command" == "free" ]]; then
                    output_var="$1" # 分配或释放的变量名
                fi
                shift
                ;;
        esac
    done

    # 检查命令类型
    if [[ -z "$command" ]]; then
        echo "Usage: attacher <setup|call|destroy|alloc|free> [options]"
        return 1
    fi

    # 根据命令类型调用对应的函数
    case $command in
        setup)
            attacher_setup
            ;;
        call)
            if [[ -z "$func_str" ]]; then
                echo "Error: func_str is required for 'call' command."
                return 1
            fi
            attacher_call "$output_var"
            ;;
        alloc)
            if [[ -z "$output_var" ]]; then
                echo "Error: output_var is required for 'alloc' command."
                return 1
            fi
            echo "Allocate memory for $output_var success!"
            node_cli --log-level=$log_level call_bus $node_id@{"malloc(10, \"null\", $output_var, 128)"}
            ;;
        free)
            if [[ -z "$output_var" ]]; then
                echo "Error: output_var is required for 'free' command."
                return 1
            fi
            echo "Release memory for $output_var success!"
            node_cli --log-level=$log_level call_bus $node_id@{"mfree($output_var)"}
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

# attacher "$@"