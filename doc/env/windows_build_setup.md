# 在 Windows 上编译 libobject

> 适用版本：libobject 2.15.2.x
> 工具链：MSYS2 + MinGW-w64 (GCC) + CMake + Ninja
>
> **分支说明**：Windows 移植的改动都在 `windows-port` 分支上，上游 `master` 保持干净
> （Linux 优先）。要用 Windows 移植就 checkout `windows-port`：
>
> ```bash
> git checkout windows-port
> ```

## 1. 安装工具链

libobject 的 Windows 构建用 **MinGW-w64(GCC)**，不是 MSVC（源码里有 `__attribute__((constructor))`、
内联汇编、`-Wno-*` 等 GCC 专有写法）。MSYS2 是 Windows 上最省事的 MinGW 发行版。

```powershell
winget install --id MSYS2.MSYS2 -e --accept-source-agreements --accept-package-agreements
```

装完后打开 **MSYS2 MINGW64** 终端（不是 MSYS 终端），安装依赖：

```bash
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja \
                 mingw-w64-x86_64-zlib mingw-w64-x86_64-xz mingw-w64-x86_64-bzip2 \
                 mingw-w64-x86_64-tools
```

| 包 | 用途 |
| --- | --- |
| `mingw-w64-x86_64-gcc` | 编译器 + 头文件 + `libwinpthread` 等运行库 |
| `mingw-w64-x86_64-cmake` | CMake（原生 Windows 版，路径空间和 GCC 一致） |
| `mingw-w64-x86_64-ninja` | 构建后端（原生，比 make 快很多） |
| `mingw-w64-x86_64-zlib` / `-xz` / `-bzip2` | 压缩库（zip/7z/squashfs/gz/bz2/lzma 需要） |
| `mingw-w64-x86_64-tools` | `gendef`/`dlltool`，用来给 `libregex.dll` 生成导入库 |

> **必须用 MINGW64 终端**：MSYS 终端里的 cmake 是 Cygwin 版，它和原生 gcc 的路径格式
> 不一致，会生成损坏的依赖文件（`make: *** 多个目标匹配`）。

## 2. 目录约定

`mk/windows.x86_64.toolchain.cmake` 会按下面的顺序自动探测 MinGW 根目录：

1. `-DMINGW_ROOT=<path>`
2. `C:/mingw64`（MinGW-w64 经典安装布局）
3. `C:/msys64/mingw64`（MSYS2 MINGW64）
4. `C:/msys64/ucrt64`（MSYS2 UCRT64）

用 MSYS2 的默认安装（`C:\msys64`）时，MSYS2 的 `mingw64` 目录在 `C:\msys64\mingw64`，
和工具链文件里的 `C:/mingw64` 对不上。两种办法：

- **建个目录联接**（推荐，不用改仓库）：

  ```cmd
  mklink /J C:\mingw64 C:\msys64\mingw64
  ```

- 或者构建时显式指定：

  ```bash
  cmake -DCMAKE_TOOLCHAIN_FILE=mk/windows.x86_64.toolchain.cmake -DMINGW_ROOT=C:/msys64/mingw64 ...
  ```

## 3. 编译

```bash
cd /c/path/to/libobject
./devops.sh build --platform=windows --arch=x86_64
```

产物：

```text
sysroot/windows/x86_64/
├── bin/          xtools.exe + 运行时 DLL(可以直接整个目录拷走运行)
├── lib/          libobject-*.dll/.a + 导入库
└── include/      头文件
```

`bin` 目录里的 DLL 是按 exe 的导入表自动收集的（`scripts/tool_cmd.sh`），
所以直接拷贝 `bin` 目录到别的机器也能跑。

CMake 4.x 需要 `CMAKE_POLICY_VERSION_MINIMUM=3.5`（旧工程里的 `cmake_minimum_required(VERSION 3.0)`
在 CMake 4 会被拒绝）：

```bash
export CMAKE_POLICY_VERSION_MINIMUM=3.5
```

## 4. 跑测试

```bash
./sysroot/windows/x86_64/bin/xtools.exe --log-type=0 --log-level=0x16 mockery -f all
```

测试用到的外部工具（`7za` / `unzip` / `bzip2` / `mksquashfs`）装在 MSYS2 的
`/usr/bin`（`7za` 在 `/usr/lib/p7zip`）：

```bash
pacman -S --needed unzip bzip2 squashfs-tools p7zip
cp /usr/lib/p7zip/7za.exe /usr/lib/p7zip/7z.exe /usr/bin/   # 让 cmd.exe 也能找到
```

并把 `C:\msys64\usr\bin`、`C:\msys64\usr\lib\p7zip` 加到 **Windows 的 PATH**
（测试用例里是 `system()`/`popen()` 调用，走的是 cmd.exe）。

### 当前结果（windows-port 分支）

```text
压缩相关用例: 65/65 全部通过
  (7z 4, zip 16, tbz2 3, squashfs 4, deflate 3, gz 2, bz2 2, lzma 2, z 17,
   tar 4, tgz 2, archive_cli 6)
全量用例: 204 通过 / 11 失败
```

11 个失败都是**非压缩类**的已知问题：`test_bn_div_case3`、`test_bn_mul_case1`（大数）、
`test_fs_tree`、`test_fs_tree_with_depth`（目录树）、`test_fshell_load`、
`test_fshell_load_and_get_func_addr`（动态库加载）、`test_http_upload`、
`test_http_upload_to_path`（http 上传）。

Windows 上被排除的用例（见 `tests/CMakeLists.txt`）：

| 用例 | 原因 |
| --- | --- |
| `tests/stub/*` | 运行时打桩的 hook 是 32 位相对跳转，跨 DLL 跳不过去 |
| `test_p2p` / `test_vpn` | 事件循环的 udp 处理没适配；vpn 需要 TUN 设备（Windows 只有 TAP） |
| `test_bus` | client 销毁时事件线程和主线程会同时改 producer 的 worker 链路，有竞态 |
| `tests/concurrent/*` | 事件/worker 的时序和 linux 不一样，偶发崩溃 |
| `test_node` | node 服务在 windows 上跑不起来，退出时会把 worker 链表搞坏，连带整个测试进程崩 |

> 测试资源（`tests/**/res/**`）在仓库里是 LF，`.gitattributes` 里强制 `eol=lf`：
> 压缩/解压的用例是拿解出来的内容跟参考文件**逐字节**比对的，CRLF 会让它们失败。
> 切分支后如果资源变成了 CRLF，删掉工作区的文件再 `git checkout-index -a -f` 重新签出。

## 5. Windows 上暂不支持的功能

| 功能 | 状态 | 说明 |
| --- | --- | --- |
| `stub`（运行时打桩/hook） | 不支持 | 跳转指令是 32 位相对跳转，跨 DLL 跳不过去，测试用例已排除 |
| `p2p` / `stun` | 不支持 | 事件循环的 udp 处理还没适配，测试用例已排除 |
| `vpn`（TUN 设备） | 存根 | Windows 用的是 TAP 驱动，`os/window/Tun.c` 目前只返回错误 |
| `attach`（进程注入） | 仅 Linux | 依赖 ptrace |
| `database`(mysql) | 仅 Linux | 依赖 mysql 客户端库 |

## 6. 从 Linux 交叉编译到 Windows

`mk/windows.x86_64.cross.toolchain.cmake` 走的是 Linux 上的 `x86_64-w64-mingw32-gcc`：

```bash
sudo apt install mingw-w64
./devops.sh build --platform=windows --arch=x86_64
```

交叉编译和原生编译共用同一套 `mk/windows.cmake` 配置，模块列表、编译选项完全一致。
