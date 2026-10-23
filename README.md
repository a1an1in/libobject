# libobject

[TOC]

- [Intro](#intro)
- [Testing with mockery](#testing-with-mockery)
- [Installation](#installation)
  - [Build Instructions](#build-instructions)
    - [Linux](#linux)
    - [Windows](#windows)
    - [Android](#android)
    - [Mac OS](#mac-os)
  - [Release Instructions](#release-instructions)
    - [Linux](#linux-1)
    - [Windows](#windows-1)
    - [Android](#android-1)
    - [Other Platforms](#other-platforms)
  - [Deployment Instructions](#deployment-instructions)
    - [Linux Deployment](#linux-deployment)
    - [Android Deployment](#android-deployment)
- [User Guide](#user-guide)
- [Contact](#contact)
- [License](#license)

## Intro
This library is designed for object-oriented programming using the C language. It currently contains core, net, concurrent, and ui (not completed) modules. Each module provides demos that are easy to understand and use.

## Installation

### Build Instructions

构建入口为 `devops.py`（纯 Python 标准库实现，需 Python 3.8+，跨 Linux/macOS/Windows），命令格式：`python3 devops.py <command> [options]`。

**Docker 构建**：`build`/`release` 支持 `--docker` 选项，在开发容器（`docker/dockerfile.dev` 构建的 `libobject-dev` 镜像，含 Linux/Android 全套工具链，镜像不存在时会自动构建）内执行编译，宿主机无需安装依赖：

```bash
python3 devops.py build --platform=linux --arch=x86_64 --docker
python3 devops.py build --platform=android --arch=arm64-v8a --docker
python3 devops.py release --platform=linux --arch=x86_64 --docker
```

也可以直接进入开发容器的交互 shell（源码挂载在 `/work`，容器内可手动执行任意构建命令）：

```bash
python3 devops.py shell
```

#### Linux

To build the project on Linux, you can specify the architecture or use the default (x86_64):

```bash
# Default architecture (x86_64)
python3 devops.py build --platform=linux

# Specify architecture (x86_64, arm or aarch64)
python3 devops.py build --platform=linux --arch=x86_64
python3 devops.py build --platform=linux --arch=arm
python3 devops.py build --platform=linux --arch=aarch64
```

The build output will be placed in the `sysroot/linux/<architecture>` directory, where `<architecture>` is one of `x86_64`, `arm` (32-bit) or `aarch64` (64-bit).

**ARM 架构说明**：
- `arm`：32 位 ARM（armv7-a，硬浮点），使用 `arm-linux-gnueabihf` 交叉编译工具链，输出到 `sysroot/linux/arm`。
- `aarch64`（或 `arm64`）：64 位 ARM，使用 `aarch64-linux-gnu` 交叉编译工具链，输出到 `sysroot/linux/aarch64`。

两种 ARM 架构使用独立的工具链文件（`mk/linux-arm.toolchain.cmake` 与 `mk/linux-aarch64.toolchain.cmake`）和独立的构建/输出目录，可共存互不影响。

#### Windows

To build the project on Windows, you can use either **native Windows compilation** or **Linux cross-compilation**.

- **Native Windows Compilation**:
  Run the following command on a Windows host (MSYS2 + MinGW-w64 toolchain):
  ```bash
  python3 devops.py build --platform=windows --arch=x86_64
  ```
  For details on setting up the Windows toolchain, refer to the [Windows Build Setup Guide](./doc/env/windows_build_setup.md).

- **Linux Cross-Compilation for Windows**:
  Set up the cross-compilation environment on a Linux host and run:
  ```bash
  python3 devops.py build --platform=windows --arch=x86_64
  ```
  For details on setting up the Linux cross-compilation environment, refer to the [Linux Cross-Compilation Setup Guide](./doc/env/linux_cross_compile_windows_setup.md).

#### Android

To build the project for Android, use the following command:

```bash
python3 devops.py build --platform=android --arch=arm64-v8a
```

Before building for Android, ensure that the Android NDK is installed. Follow the [Installing Android NDK on Ubuntu](./doc/env/install_android_ndk.md) guide for detailed instructions.

#### Mac OS

To build the project on macOS, use the following command:

```bash
python3 devops.py build --platform=mac
```

### Release Instructions

#### Linux
**Release package**

To create a release package for Linux, you can specify the architecture or use the default (x86_64):

```bash
# Default architecture (x86_64)
python3 devops.py release --platform=linux

# Specify architecture (x86_64, arm or aarch64)
python3 devops.py release --platform=linux --arch=x86_64
python3 devops.py release --platform=linux --arch=arm
python3 devops.py release --platform=linux --arch=aarch64
```

The release package will be created in the `packages` directory with a name like `xtools_linux_<architecture>_v<version>.tar.gz`.

**Release docker image**
```
sudo python3 devops.py docker --build=fruit-noded --dockerfile=dockerfile.noded --package-path=./packages/xtools_linux_x86_64_v2.15.0.241.tar.gz
sudo docker run -d --name fruit-noded -p 12345:12345 fruit-noded
./sysroot/linux/x86_64/bin/xtools node_cli --host=0.0.0.0 --service=12345 lookup all 
```

#### Windows

To create a release package for Windows, you can specify the architecture (default is x86_64):

```bash
# Default architecture (x86_64)
python3 devops.py release --platform=windows

# Specify architecture (x86_64)
python3 devops.py release --platform=windows --arch=x86_64
```

The release package will be created in the `packages` directory with a name like `xtools_windows_<architecture>_v<version>.zip`.

#### Android

To create a release package for Android, specify the architecture (default is `arm64-v8a`):

```bash
# Default architecture (arm64-v8a)
python3 devops.py release --platform=android

# Specify architecture (e.g., arm64-v8a)
python3 devops.py release --platform=android --arch=arm64-v8a
```

The release package will be created in the `packages` directory with a name like `xtools_android_<architecture>_v<version>.zip`.

#### Other Platforms

To create a release package for other platforms, use the following command:

```bash
# Replace <platform> with mac or android
python3 devops.py release --platform=<platform>
```

### Deployment Instructions

To deploy the project, use the `develop.sh` script. This script simplifies the deployment process for various platforms.
#### Linux Deployment

To deploy the project to a Linux server, use the following command:

```bash
python3 devops.py deploy -p=linux --host=<server_ip> --package-path=<path_to_package>
```
This command connects to the deployed service on the specified host (`139.159.231.27`) and service port (`12345`) to perform a lookup operation for all available nodes.
- `<server_ip>`: Replace with the IP address of the target Linux server (e.g., `139.159.231.27`).
- `<path_to_package>`: Replace with the path to the release package (e.g., `./packages/xtools_linux_x86_64_v2.14.0.125.tar.gz`).

##### Example

```bash
python3 devops.py deploy -p=linux --host=139.159.231.27 --package-path=./packages/xtools_linux_x86_64_v2.15.0.153.tar.gz
```

This command will deploy the specified package to the Linux server at `139.159.231.27`.

#### Verification

After deploying the project to a Linux server, you can verify the deployment using the following command:

```bash
./sysroot/linux/x86_64/bin/xtools node_cli --host="139.159.231.27" --service="12345" lookup all
```

The `xtools` binary also supports log-related options to control log output. The `--log-type` option overrides `debugger:type` in `~/.xtools/dbg.ini` (0 = console/shell print, 1 = log file, 2 = network), and `--log-level` sets the display level. These options must be followed by a subcommand:

```bash
# Print logs to the shell (no need to modify dbg.ini)
./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 mockery -f test_uio_fpga
```

#### Android Deployment Example

To deploy the project to an Android device, use the following command:

```bash
python3 devops.py deploy -p=android --package-path=<path_to_package>
```

- `<path_to_package>`: Replace with the path to the release package (e.g., `./packages/xtools_android_arm64-v8a_v2.14.0.125.zip`).

##### Example

```bash
python3 devops.py deploy -p=android --package-path=./packages/xtools_android_arm64-v8a_v2.15.0.128.tar.gz
```

This command will deploy the specified package to the connected Android device.

## Testing with mockery

Unit tests live in `tests/<module>/test_*.c` and are registered with `REGISTER_TEST_FUNC(test_xxx)` /
`REGISTER_TEST_CMD(...)`, compiled into `object-tests`, and linked into the `xtools` binary.

### Recommended: the `devops.py test` command

Build first, then run tests with the devops front-end — it resolves the right test backend for
your host, propagates `--log-level` / `--log-type` to `xtools`, and returns a non-zero exit code
when any test fails (exit code is real; the whole run aborts with status 1 on test failure):

```bash
# linux tests: native on a Linux host, inside the dev container via docker otherwise
python devops.py test -p linux --log-level=0x14 --log-type=0 -f test
python devops.py test -p linux --log-level=0x14 
python devops.py test -p linux -f test_archive_

# windows tests: native (host is Windows)
python devops.py test -p windows --log-level=0x14 --log-type=0 -f test
python devops.py test -p windows  --log-level=0x14 

# run everything (linux + windows), only a prefix subset, or with verbose logs
python devops.py test
```

Notes:

- `-p` accepts `linux`, `windows`, or `all` (default: `all`). On a non-Linux host the linux
  suite runs inside the dev container (`docker/dockerfile.dev`, image `libobject-dev`, built
  automatically if missing); windows tests always run natively on a Windows host.
- `-f <name>` matches registered test **functions** by prefix (`strncmp`): `-f test_archive_`
  runs every `test_archive_*` function, `-f test` runs all of them.
- Tests must run **from the project root** (they use relative paths like `./tests/...`);
  `devops.py test` does that for you.

### Manual: calling mockery directly

For debugging a single test it can be faster to invoke `xtools` yourself (same options):

```bash
# Run a single test function (or prefix-match all test_xxx* functions)
./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 mockery -f test_uio_fpga

# Run every test whose name starts with a prefix, e.g. the whole archive test set
./sysroot/linux/x86_64/bin/xtools --log-type=0 --log-level=0x16 mockery -f test_archive_

# Run all registered test functions
./sysroot/linux/x86_64/bin/xtools mockery -f all

# Without -f, mockery <name> runs registered test commands (REGISTER_TEST_CMD)
./sysroot/linux/x86_64/bin/xtools mockery test_bus_server
```

Log options are shared with the devops command: `--log-type` (0 = console, 1 = log file,
2 = network) and `--log-level` (debug level / business mask, e.g. `0x6` opens all businesses
at level 6). More examples: [`doc/test_cmds.md`](doc/test_cmds.md).

## User Guide

The following modules are included in the library. Click on a module name to view its detailed documentation:

- [Archive Module](./doc/archive/README.md): Provides archive (packing/unpacking) support, including tar, zip, tgz, tbz2, 7z and squashfs formats.
- [Argument Module](./doc/argument/README.md): Handles command-line arguments and parameter parsing.
- [Attacher Module](./doc/attacher/README.md): Platform-specific module for resource attachment (not available on ARM).
- [Concurrent Module](./doc/concurrent/README.md): Includes utilities for multithreading and synchronization, such as thread pools and mutexes.
- [Core Module](./doc/core/README.md): Provides the foundation for object-oriented programming in C, including inheritance, polymorphism, and encapsulation.
- [Crypto Module](./doc/crypto/README.md): Provides cryptographic utilities, such as encryption and decryption.
- [Database Module](./doc/database/README.md): Platform-specific module for database operations (not available on ARM).
- [Board Module](./doc/drivers/README.md): Provides board-specific abstractions for hardware drivers (HAL) and reusable hardware services (Service).
- [Encoding Module](./doc/encoding/README.md): Handles data encoding and decoding, such as Base64 and JSON.
- [Event Module](./doc/event/README.md): Supports event-driven programming models, including event registration, dispatching, and handling.
- [Net Module](./doc/net/README.md): Offers networking capabilities, such as TCP/UDP socket programming and HTTP support.
- [Node Module](./doc/node/README.md): Manages distributed nodes and their communication.
- [Scripts Module](./doc/scripts/README.md): Provides scripting utilities for automation and integration.
- [Stub Module](./doc/stub/README.md): Handles stub generation for remote procedure calls (RPC).
- [UI Module](./doc/ui/README.md): Tools for building graphical user interfaces (under development).

## Contact
The latest version of the library is available at [GitHub](https://github.com/a1an1in/libobject).  
If you encounter any issues or have questions, feel free to contact me via email at `a1an1in@sina.com`.

## License
This software is licensed under the BSD 3-Clause license.
Copyright (c) 2015-2020, alan lin <a1an1in@sina.com>.
