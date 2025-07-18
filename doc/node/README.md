# Node Operations Guide

## Table of Contents

- [1. Deploying the Node Service](#1-deploying-the-node-service)
    - [1.1 Deploying on Linux](#11-deploying-on-linux)
    - [1.2 Deploying on Android](#12-deploying-on-android)
- [2. Node CLI Operations](#2-node-cli-operations)
    - [2.1 Querying Node ID](#21-querying-node-id)
    - [2.2 File Operations](#22-file-operations)
    - [2.3 Command Execution](#23-command-execution)
    - [2.5 Migrating `noded`](#25-migrating-noded)
        - [2.5.1 Allocating Command and Configuration](#251-allocating-command-and-configuration)
        - [2.5.2 Setting Configuration](#252-setting-configuration)
        - [2.5.3 Applying Configuration](#253-applying-configuration)
        - [2.5.4 Getting Global Address](#254-getting-global-address)
        - [2.5.5 Freeing Allocated Memory](#255-freeing-allocated-memory)
    - [2.6 Memory Operations](#26-memory-operations)
    - [2.7 Node Upgrade Operations](#27-node-upgrade-operations)
    - [2.8 Stub and Hook Management](#28-stub-and-hook-management)
        - [2.8.1 Allocating Stub and Variables](#281-allocating-stub-and-variables)
        - [2.8.2 Adding Stub Hooks](#282-adding-stub-hooks)
        - [2.8.3 Testing Function with Stub Hooks](#283-testing-function-with-stub-hooks)
        - [2.8.4 Removing Stub Hooks](#284-removing-stub-hooks)
        - [2.8.5 Testing Function Without Stub Hooks](#285-testing-function-without-stub-hooks)
        - [2.8.6 Freeing Stub and Variables](#286-freeing-stub-and-variables)
    - [2.9 Attacher Operations](#29-attacher-operations)
        - [2.9.1 Starting the Service and Testing the Process](#291-starting-the-service-and-testing-the-process)
        - [2.9.2 Opening the Attacher](#292-opening-the-attacher)
        - [2.9.3 Invoking Target Methods](#293-invoking-target-methods)
        - [2.9.4 Simplifying Attacher Operations with Scripts](#294-simplifying-attacher-operations-with-scripts)
    - [2.10 Windows Testing](#210-windows-testing)
        - [2.10.1 Starting the Node Service](#2101-starting-the-node-service)
        - [2.10.2 Node CLI Examples](#2102-node-cli-examples)

---
The node module manages distributed nodes and their communication.

## 1. Deploying the Node Service

### 1.1 Deploying on Linux
To deploy the Node Service on a Linux system, use the following commands:
```bash
./devops.sh build --platform=linux
./devops.sh release -p=linux
./devops.sh deploy -p=linux --host=139.159.231.27 --package-path=./packages/xtools_linux_x86_64_v2.15.0.187.tar.gz
./sysroot/linux/x86_64/bin/xtools mockery --log-level=0x14 -f test_node
./sysroot/linux/x86_64/bin/xtools node --log-level=0x30016 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/x86_64/bin/xtools node --log-level=0x20016 --host=139.159.231.27 --service=12345
```

### 1.2 Deploying on Android
To deploy the Node Service on an Android device, use the following commands:
```bash
./devops.sh build --platform=android --arch=arm64-v8a
./devops.sh release --platform=android --arch=arm64-v8a
./devops.sh deploy -p=android --package-path=./packages/xtools_android_arm64-v8a_v2.15.0.188.tar.gz
adb shell 
cd  /data/local/tmp/.xtools/
export LD_LIBRARY_PATH=/data/local/tmp/.xtools/sysroot/lib:$LD_LIBRARY_PATH
./sysroot/bin/xtools node --log-level=0x30015 --host=139.159.231.27 --service=12345
./sysroot/bin/xtools --log-level=0x20017 node -h
nohup ./sysroot/bin/xtools node --log-level=0x30015 --host=139.159.231.27 --service=12345 >/data/local/tmp/.xtools/logs 2>&1 &
./sysroot/linux/x86_64/bin/xtools node_cli --host="139.159.231.27" --service="12345" lookup all
```

---

## 2. Node CLI Operations

This section provides detailed examples of using `node_cli` to interact with nodes.

### 2.1 Querying Node ID
Retrieve the node ID by querying all available nodes.
```bash
export HOST="www.yunisona.top"
export SERVICE="12345"
node_cli() {
    local HOST="${HOST:-127.0.0.1}"
    local SERVICE="${SERVICE:-12345}"

    if [[ "$(uname -s)" == "Linux" ]]; then
        ND_CLI="./sysroot/linux/x86_64/bin/xtools node_cli"
    elif [[ "$(uname -s)" == "MINGW"* || "$(uname -s)" == "CYGWIN"* || "$(uname -s)" == "MSYS"* ]]; then
        ND_CLI="./sysroot/windows/x86_64/bin/xtools.exe node_cli"
    else
        echo "Unsupported OS: $(uname -s)"
        return 1
    fi

    $ND_CLI --host="$HOST" --service="$SERVICE" "$@"
}

或者：
source ./sysroot/linux/x86_64/bin/node_cli.sh www.yunisona.top 12345

node_id=$(node_cli lookup all | grep "index:2" | awk -F'id:' '{print $2}' | awk -F',' '{print $1}' | tr -d ' ')
echo $node_id
node_cli lookup all

node_cli --log-level=0x15 call_cmd $node_id@{"ls -l"}
```

### 2.2 File Operations
List and copy files on the node.
```bash
# List all packages in the node's package directory
node_cli flist $node_id@/root/.xtools/packages

# List all files in the test node directory
node_cli flist $node_id@./tests/node/

# Copy a file to the node
node_cli fcopy ./tests/node/res/test_node.txt "$node_id@./tests/node/output/write/test_node.txt"
```

### 2.3 Command Execution
Run shell commands on the node.
```bash
# List files in the current directory
node_cli --log-level=0x30015 call_cmd $node_id@{"ls -l"}

# Print the current working directory
node_cli call_cmd $node_id@{"pwd"}
```

### 2.5 Migrating `noded`
This subsection demonstrates how to migrate `noded` using `node_cli`.

#### 2.5.1 Allocating Command and Configuration
Allocate memory for the `node_command` and `node_config`.
```bash
node_cli call_bus $node_id@{"malloc(10, \"null\", #node_command, 8)"}
node_cli call_bus $node_id@{"malloc(10, \"null\", #node_config, 128)"}
```

#### 2.5.2 Setting Configuration
Set the configuration for the node.
```bash
node_cli mset $node_id@#node_config{0-127} "{\"log-level\": \"0x30015\"}"
```

#### 2.5.3 Applying Configuration
Apply the configuration to the `node_command`.
```bash
node_cli call_fsh $node_id@{"node_command_config(*#node_command, #node_config)"}
```

#### 2.5.4 Getting Global Address
Retrieve the global address of the `node_command`.
```bash
node_cli call_fsh $node_id@{"node_command_get_global_addr(#node_command)"}
```

#### 2.5.5 Freeing Allocated Memory
Free the allocated memory for `node_config` and `node_command`.
```bash
node_cli call_bus $node_id@{"mfree(#node_config)"}
node_cli call_bus $node_id@{"mfree(#node_command)"}
```

### 2.6 Memory Operations
Allocate and manage memory on the node.
```bash
node_cli call_bus $node_id@{"malloc(10, \"null\", #node_config, 128)"}
node_cli mset $node_id@#node_config{0-127} "{\"log-level\": \"0x30016\",\"host\": \"139.159.231.27\",\"service\": \"12345\"}"
node_cli mget $node_id@#node_config /100s
node_cli call_bus $node_id@{"mfree(#node_config)"}
```

### 2.7 Node Upgrade Operations
Perform a node upgrade using specific commands.
```bash
node_cli call_bus $node_id@{"malloc(10, \"null\", #node_command, 8)"}
node_cli call_fsh $node_id@{"node_command_get_global_addr(#node_command)"}
node_cli call_fsh $node_id@{"node_command_upgrade(*#node_command)"}
node_cli call_bus $node_id@{"mfree(#node_command)"}
```

### 2.8 Stub and Hook Management
Manage stubs and hooks on the node.

#### 2.8.1 Allocating Stub and Variables
Allocate memory for a stub and a variable.
```bash
node_cli call_bus "$node_id@malloc(12, \"null\", #test_stub_name1, 0)"
node_cli call_bus "$node_id@malloc(10, \"null\", #test_v1, 8)"
```

#### 2.8.2 Adding Stub Hooks
Add hooks to a stub for intercepting function calls.
```bash
node_cli call_fsh "$node_id@fsh_add_stub_hooks(#test_stub_name1, \"test_func\", \"test_print_inbound\", \"test_target_func\", \"test_print_outbound\", 7)"
```

#### 2.8.3 Testing Function with Stub Hooks
Invoke a function with the stub hooks applied.
```bash
node_cli call_fsh "$node_id@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
```

#### 2.8.4 Removing Stub Hooks
Remove the hooks from the stub.
```bash
node_cli call_fsh "$node_id@fsh_remove_stub_hooks(#test_stub_name1)"
```

#### 2.8.5 Testing Function Without Stub Hooks
Invoke the function again after removing the stub hooks.
```bash
node_cli call_fsh "$node_id@test_func(1, 2, 3, 4, 5, 6, #test_v1)"
```

#### 2.8.6 Freeing Stub and Variables
Free the allocated memory for the stub and variable.
```bash
node_cli call_bus "$node_id@mfree(#test_stub_name1)"
node_cli call_bus "$node_id@mfree(#test_v1)"
```

### 2.9 Attacher Operations

#### 2.9.1 Starting the Service and Testing the Process
```bash
./sysroot/linux/x86_64/bin/test-process
pid=$(ps aux | grep test-process | grep -v grep | awk '{print $2}')
```

#### 2.9.2 Opening the Attacher
```bash
node_cli call_bus $node_id@{"malloc(13, \"UnixAttacher\", #test_attacher, 0)"}
node_cli call_obj $node_id@{"attach(#test_attacher, $pid)"}
node_cli call_obj $node_id@{"init(#test_attacher)"}
```

#### 2.9.3 Invoking Target Methods
```bash
node_cli call_bus $node_id@{"malloc(10, \"null\", #test_func_str, 128)"}
node_cli mset $node_id@#test_func_str{0-127} "attacher_test_with_pointer_arg(0x1234, \"test2\")"
node_cli call_obj $node_id@{"call(#test_attacher, 0, #test_func_str, 0)"}
node_cli call_bus $node_id@{"mfree(#test_attacher)"}
```

#### 2.9.4 Simplifying Attacher Operations with Scripts
```bash
source ./sysroot/linux/x86_64/bin/attacher.sh
attacher setup --filter="fd:10" --app-name="test-process"
attacher alloc "#test_result"
attacher call --log-level=0x20014 "attacher_test_with_pointer_arg(0x1234, \"test2\")" "#test_result"
attacher free "#test_result"
attacher destroy
```

### 2.10 Windows Testing

#### 2.10.1 Starting the Node Service
```bash
./sysroot/windows/x86_64/bin/xtools node --log-level=0x15 --host=139.159.231.27 --service=12345
```

#### 2.10.2 Node CLI Examples
```bash
node_cli flist $node_id@/root/.xtools/packages
node_cli flist $node_id@./tests/node/
node_cli call_cmd $node_id@{"ls -l"}
node_cli lookup all
```