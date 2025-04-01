# libobject

[TOC]

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

To build the project on Windows, use the following command:

```bash
./devops.sh build --platform=windows
```

#### Mac OS

To build the project on macOS, use the following command:

```bash
./devops.sh build --platform=mac
```

#### Android

To build the project for Android, use the following command:

```bash
export NDK_ROOT=/opt/android-ndk
./devops.sh build --platform=android --arch=arm64-v8a
```

Before building for Android, ensure that the Android NDK is installed. Follow the [Installing Android NDK on Ubuntu](./doc/env/install_android_ndk.md) guide for detailed instructions.

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

#### Other Platforms

To create a release package for other platforms, use the following command:

```bash
# Replace <platform> with mac, android, or windows
./devops.sh release --platform=<platform>
```

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
