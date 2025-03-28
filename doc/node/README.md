# Node Operations Guide

The node module manages distributed nodes and their communication.

---

## 1. Deploying the Node Service

### 1.1 Starting the Node Service
```bash
./sysroot/linux/bin/xtools node --log-level=0x30016 --host=127.0.0.1 --service=12345 --deamon=t
./sysroot/linux/bin/xtools node --log-level=0x20016 --host=127.0.0.1 --service=12345
```

---

## 2. Node CLI Operations

This section provides detailed examples of using `node_cli` to interact with nodes.

### 2.1 Querying Node ID
Retrieve the node ID by querying all available nodes.
```bash
node_cli() {
    # Detect the operating system type
    if [[ "$(uname -s)" == "Linux" ]]; then
        ND_CLI="./sysroot/linux/bin/xtools node_cli"
    elif [[ "$(uname -s)" == "MINGW"* || "$(uname -s)" == "CYGWIN"* || "$(uname -s)" == "MSYS"* ]]; then
        ND_CLI="./sysroot/windows/bin/xtools.exe node_cli"
    else
        echo "Unsupported OS: $(uname -s)"
        return 1
    fi

    # Execute the command
    $ND_CLI --host="127.0.0.1" --service="12345" "$@"
}

node_id=$(node_cli lookup all | grep "index:2" | awk -F'id:' '{print $2}' | awk -F',' '{print $1}' | tr -d ' ')
node_cli lookup all
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
node_cli call_cmd $node_id@{"ls -l"}

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
./sysroot/linux/bin/test-process
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
source ./sysroot/linux/bin/attacher.sh
attacher setup --filter="fd:10" --app-name="test-process"
attacher alloc "#test_result"
attacher call --log-level=0x20014 "attacher_test_with_pointer_arg(0x1234, \"test2\")" "#test_result"
attacher free "#test_result"
attacher destroy
```

### 2.10 Windows Testing

#### 2.10.1 Starting the Node Service
```bash
./sysroot/windows/bin/xtools node --log-level=0x15 --host=139.159.231.27 --service=12345
```

#### 2.10.2 Node CLI Examples
```bash
node_cli flist $node_id@/root/.xtools/packages
node_cli flist $node_id@./tests/node/
node_cli call_cmd $node_id@{"ls -l"}
node_cli lookup all
```