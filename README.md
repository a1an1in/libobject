# libobject

[TOC]

- [Intro](#intro)
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

#### Linux

To build the project on Linux, you can specify the architecture or use the default (x86_64):

```bash
# Default architecture (x86_64)
./devops.sh build --platform=linux

# Specify architecture (x86_64 or arm)
./devops.sh build --platform=linux --arch=x86_64
./devops.sh build --platform=linux --arch=arm
```

The build output will be placed in the `sysroot/linux/<architecture>` directory, where `<architecture>` is either `x86_64` or `arm`.

#### Windows

To build the project on Windows, you can use either **native Windows compilation** or **Linux cross-compilation**.

- **Native Windows Compilation**:
  Run the following command on a Windows host:
  ```bash
  ./devops.sh build --platform=windows --arch=x86_64
  ```

- **Linux Cross-Compilation for Windows**:
  Set up the cross-compilation environment on a Linux host and run:
  ```bash
  ./devops.sh build --platform=windows --arch=x86_64
  ```
  For details on setting up the Linux cross-compilation environment, refer to the [Linux Cross-Compilation Setup Guide](./doc/env/linux_cross_compile_windows_setup.md).

#### Android

To build the project for Android, use the following command:

```bash
./devops.sh build --platform=android --arch=arm64-v8a
```

Before building for Android, ensure that the Android NDK is installed. Follow the [Installing Android NDK on Ubuntu](./doc/env/install_android_ndk.md) guide for detailed instructions.

#### Mac OS

To build the project on macOS, use the following command:

```bash
./devops.sh build --platform=mac
```

### Release Instructions

#### Linux

To create a release package for Linux, you can specify the architecture or use the default (x86_64):

```bash
# Default architecture (x86_64)
./devops.sh release --platform=linux

# Specify architecture (x86_64 or arm)
./devops.sh release --platform=linux --arch=x86_64
./devops.sh release --platform=linux --arch=arm
```

The release package will be created in the `packages` directory with a name like `xtools_linux_<architecture>_v<version>.tar.gz`.

#### Windows

To create a release package for Windows, you can specify the architecture (default is x86_64):

```bash
# Default architecture (x86_64)
./devops.sh release --platform=windows

# Specify architecture (x86_64)
./devops.sh release --platform=windows --arch=x86_64
```

The release package will be created in the `packages` directory with a name like `xtools_windows_<architecture>_v<version>.zip`.

#### Android

To create a release package for Android, specify the architecture (default is `arm64-v8a`):

```bash
# Default architecture (arm64-v8a)
./devops.sh release --platform=android

# Specify architecture (e.g., arm64-v8a)
./devops.sh release --platform=android --arch=arm64-v8a
```

The release package will be created in the `packages` directory with a name like `xtools_android_<architecture>_v<version>.zip`.

#### Other Platforms

To create a release package for other platforms, use the following command:

```bash
# Replace <platform> with mac or android
./devops.sh release --platform=<platform>
```

### Deployment Instructions

To deploy the project, use the `develop.sh` script. This script simplifies the deployment process for various platforms.
#### Linux Deployment

To deploy the project to a Linux server, use the following command:

```bash
./devops.sh deploy -p=linux --host=<server_ip> --package-path=<path_to_package>
```
This command connects to the deployed service on the specified host (`139.159.231.27`) and service port (`12345`) to perform a lookup operation for all available nodes.
- `<server_ip>`: Replace with the IP address of the target Linux server (e.g., `139.159.231.27`).
- `<path_to_package>`: Replace with the path to the release package (e.g., `./packages/xtools_linux_x86_64_v2.14.0.125.tar.gz`).

##### Example

```bash
./devops.sh deploy -p=linux --host=139.159.231.27 --package-path=./packages/xtools_linux_x86_64_v2.15.0.153.tar.gz
```

This command will deploy the specified package to the Linux server at `139.159.231.27`.

#### Verification

After deploying the project to a Linux server, you can verify the deployment using the following command:

```bash
./sysroot/linux/x86_64/bin/xtools node_cli --host="139.159.231.27" --service="12345" lookup all
```

#### Android Deployment Example

To deploy the project to an Android device, use the following command:

```bash
./devops.sh deploy -p=android --package-path=<path_to_package>
```

- `<path_to_package>`: Replace with the path to the release package (e.g., `./packages/xtools_android_arm64-v8a_v2.14.0.125.zip`).

##### Example

```bash
./devops.sh deploy -p=android --package-path=./packages/xtools_android_arm64-v8a_v2.15.0.128.tar.gz
```

This command will deploy the specified package to the connected Android device.

### User Guide

The following modules are included in the library. Click on a module name to view its detailed documentation:

- [Argument Module](./doc/argument/README.md): Handles command-line arguments and parameter parsing.
- [Attacher Module](./doc/attacher/README.md): Platform-specific module for resource attachment (not available on ARM).
- [Concurrent Module](./doc/concurrent/README.md): Includes utilities for multithreading and synchronization, such as thread pools and mutexes.
- [Core Module](./doc/core/README.md): Provides the foundation for object-oriented programming in C, including inheritance, polymorphism, and encapsulation.
- [Crypto Module](./doc/crypto/README.md): Provides cryptographic utilities, such as encryption and decryption.
- [Database Module](./doc/database/README.md): Platform-specific module for database operations (not available on ARM).
- [Drivers Module](./doc/drivers/README.md): Provides abstractions for hardware drivers.
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
This software is licensed under the LGPL license.  
Copyright (c) 2015-2020.
