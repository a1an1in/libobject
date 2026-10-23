#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
devops 跨平台构建脚本（Python 版）。
完整移植自 devops.sh + scripts/tool_cmd.sh，仅使用 Python 标准库，
支持 Linux / macOS / Windows 运行。
"""

import argparse
import os
import platform
import re
import shutil
import subprocess
import sys
import tarfile

# 工程根目录（脚本所在目录），bash 版默认从工程根目录执行
PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))

# docker 镜像版本号（与 bash 版保持一致的硬编码）
DOCKER_IMAGE_VERSION = "2.15.0.229"

# ssh 密码（deploy 时配合 sshpass 自动应答）
SSH_PASSWORD = "Alan@123"

# 开发镜像（docker/dockerfile.dev），--docker 选项使用
DOCKER_DEV_IMAGE = "libobject-dev"
DOCKER_DEV_DOCKERFILE = os.path.join("docker", "dockerfile.dev")


class Options:
    """全局选项，对应 scripts/environment_variables.sh 中的默认值"""

    def __init__(self):
        self.help = False
        self.platform = ""
        self.install = False
        self.build = False
        self.db_ip = ""
        self.to_path = "~/.xtools/packages"
        self.package_path = ""
        self.name = ""
        self.args = ""
        self.remove = False
        self.run = False
        self.ip = ""
        self.arch = ""
        self.filter = "all"
        self.log_type = "0"
        self.log_level = "0x16"
        self.dockerfile = ""
        self.docker = False
        self.forward_args = []
        self.cmd_name = "false"


OPTION = Options()


def run(cmd, cwd=None, check=False, err_msg=None, capture=False):
    """执行外部命令，check 为 True 时失败即报错退出"""
    if isinstance(cmd, str):
        printable = cmd
    else:
        printable = " ".join(str(c) for c in cmd)
    print("+ %s" % printable)
    if capture:
        ret = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, text=True)
    else:
        ret = subprocess.run(cmd, cwd=cwd)
    if check and ret.returncode != 0:
        if err_msg:
            print(err_msg)
        sys.exit(1)
    return ret


def increase_version_tweak_number():
    """读取 CMakeLists.txt 中 project (libobject VERSION x.y.z.w)，将 w 加 1"""
    version = get_version_number()
    print("-- increase_version_tweak_number:%s" % version)
    parts = version.split(".")
    new_version = "%s.%s.%s.%d" % (parts[0], parts[1], parts[2], int(parts[3]) + 1)
    print("-- sed -i 's/%s/%s/g' CMakeLists.txt" % (version, new_version))
    path = os.path.join(PROJECT_ROOT, "CMakeLists.txt")
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()
    content = content.replace(version, new_version)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)


def get_version_number():
    """从根 CMakeLists.txt 提取工程版本号"""
    with open(os.path.join(PROJECT_ROOT, "CMakeLists.txt"), "r", encoding="utf-8") as f:
        for line in f:
            if "project (libobject VERSION" in line:
                return line.split()[3].rstrip(")")
    return ""


def get_release_package_name(prefix, version_file):
    """生成发布包名：<prefix>_<version>.tar.gz"""
    version = get_release_package_version(version_file)
    return "%s_%s.tar.gz" % (prefix, version)


def get_release_package_version(version_file):
    """从 version.h 中提取 PROJECT_VERSION（如 v2.15.2.468）"""
    with open(version_file, "r", encoding="utf-8") as f:
        for line in f:
            if "PROJECT_VERSION" in line:
                return line.split()[2].replace('"', "")
    return ""


def copy_scripts_and_includes(build_dir, sysroot_dir):
    """make 之后（非 -i 模式）复制脚本与头文件到 sysroot"""
    src_root = os.path.join(build_dir, "..", "..", "..")

    attacher = os.path.join(src_root, "doc", "attacher", "attacher.sh")
    node_cli = os.path.join(src_root, "doc", "node", "node_cli.sh")
    if not os.path.isfile(attacher):
        print("Error: attacher.sh not found in doc/attacher.")
        sys.exit(1)
    if not os.path.isfile(node_cli):
        print("Error: node_cli.sh not found in doc/node.")
        sys.exit(1)

    bin_dir = os.path.join(sysroot_dir, "bin")
    include_dir = os.path.join(sysroot_dir, "include")
    os.makedirs(bin_dir, exist_ok=True)
    os.makedirs(include_dir, exist_ok=True)

    shutil.copy(attacher, os.path.join(bin_dir, "attacher.sh"))
    print("attacher.sh has been copied to %s/bin" % sysroot_dir)
    os.chmod(os.path.join(bin_dir, "attacher.sh"), 0o755)
    shutil.copy(node_cli, os.path.join(bin_dir, "node_cli.sh"))
    print("node_cli.sh has been copied to %s/bin" % sysroot_dir)
    os.chmod(os.path.join(bin_dir, "node_cli.sh"), 0o755)

    src_include = os.path.join(src_root, "src", "include")
    for item in os.listdir(src_include):
        src = os.path.join(src_include, item)
        dst = os.path.join(include_dir, item)
        if os.path.isdir(src):
            if os.path.exists(dst):
                shutil.rmtree(dst)
            shutil.copytree(src, dst)
        else:
            shutil.copy(src, dst)
    print("include has been copied to %s/include" % sysroot_dir)


def do_build_linux():
    increase_version_tweak_number()

    # 如果未指定架构，默认使用 x86_64
    if not OPTION.arch:
        print("No architecture specified. Defaulting to x86_64.")
        OPTION.arch = "x86_64"

    # 根据架构动态设置构建目录、工具链文件和安装目录
    if OPTION.arch == "x86_64":
        print("Building for x86_64 architecture")
        build_dir = "build/linux/x86_64"
        toolchain_file = "../../../mk/linux-x86_64.toolchain.cmake"
        sysroot_dir = "../../../sysroot/linux/x86_64"
    elif OPTION.arch == "arm":
        print("Building for 32-bit ARM architecture")
        build_dir = "build/linux/arm"
        toolchain_file = "../../../mk/linux-arm.toolchain.cmake"
        sysroot_dir = "../../../sysroot/linux/arm"
    elif OPTION.arch in ("aarch64", "arm64"):
        print("Building for 64-bit ARM (aarch64) architecture")
        build_dir = "build/linux/aarch64"
        toolchain_file = "../../../mk/linux-aarch64.toolchain.cmake"
        sysroot_dir = "../../../sysroot/linux/aarch64"
    else:
        print("Error: Unsupported architecture: %s" % OPTION.arch)
        sys.exit(1)

    # 创建构建目录
    build_dir = os.path.join(PROJECT_ROOT, build_dir)
    os.makedirs(build_dir, exist_ok=True)

    # 检查工具链文件是否存在
    if not os.path.isfile(os.path.join(build_dir, toolchain_file)):
        print("Error: Toolchain file not found: %s" % toolchain_file)
        sys.exit(1)

    # 检查 CMakeLists.txt 文件是否存在于构建目录的父级
    if not os.path.isfile(os.path.join(build_dir, "../../../CMakeLists.txt")):
        print("Error: CMakeLists.txt not found in the source directory.")
        sys.exit(1)

    # 运行 CMake，加载工具链文件
    run(["cmake", "../../../", "-DPLATFORM=linux", "-DMODULE_UI=off",
         "-DCMAKE_TOOLCHAIN_FILE=%s" % toolchain_file],
        cwd=build_dir, check=True, err_msg="CMake configuration failed.")

    run(["make"], cwd=build_dir, check=True, err_msg="Make failed.")

    if OPTION.install:
        run(["make", "install"], cwd=build_dir, check=True,
            err_msg="Make install failed.")
        sys.exit(0)

    copy_scripts_and_includes(build_dir, os.path.join(build_dir, sysroot_dir))
    sys.exit(0)


def do_build_mac():
    build_dir = os.path.join(PROJECT_ROOT, "build", "mac")
    os.makedirs(build_dir, exist_ok=True)
    run(["cmake", "../..", "-DPLATFORM=mac"], cwd=build_dir, check=True,
        err_msg="CMake configuration failed.")
    run(["make"], cwd=build_dir, check=True, err_msg="Make failed.")
    # 与 bash 版一致：向 sysroot/mac/bin/xtools 偏移 160 处写入 \x07
    xtools = os.path.join(PROJECT_ROOT, "sysroot", "mac", "bin", "xtools")
    with open(xtools, "r+b") as f:
        f.seek(160)
        f.write(b"\x07")


def do_build_ios():
    build_dir = os.path.join(PROJECT_ROOT, "build", "ios")
    os.makedirs(build_dir, exist_ok=True)
    run(["cmake", "../..", "-DPLATFORM=ios", "-DIOS_OPTION_PLATFORM=SIMULATOR64",
         "-DCMAKE_TOOLCHAIN_FILE=../../mk/ios.toolchain.cmake"],
        cwd=build_dir, check=True, err_msg="CMake configuration failed.")
    # 注意：bash 版此处为 "makes"（笔误），此处保持忠实移植
    run(["makes"], cwd=build_dir)


def do_build_android():
    print("Building for Android platform")
    increase_version_tweak_number()

    # 检查 NDK 环境变量是否已设置
    ndk_root = os.environ.get("NDK_ROOT", "")
    if not ndk_root:
        ndk_root = "/opt/android-ndk"

    # 检查 NDK 路径是否有效
    if not os.path.isdir(ndk_root):
        print("Error: NDK_ROOT path '%s' does not exist. Please check your NDK installation." % ndk_root)
        sys.exit(1)

    # 设置默认架构
    if not OPTION.arch:
        print("No architecture specified. Defaulting to arm64-v8a.")
        OPTION.arch = "arm64-v8a"

    # 设置构建目录和安装目录
    os.makedirs(os.path.join(PROJECT_ROOT, "sysroot", "android", OPTION.arch), exist_ok=True)
    build_dir = os.path.join(PROJECT_ROOT, "build", "android", OPTION.arch)
    sysroot_dir = os.path.realpath(os.path.join(PROJECT_ROOT, "sysroot", "android", OPTION.arch))

    # 创建构建目录
    os.makedirs(build_dir, exist_ok=True)

    # 检查 CMakeLists.txt 文件是否存在
    if not os.path.isfile(os.path.join(build_dir, "../../../CMakeLists.txt")):
        print("Error: CMakeLists.txt not found in the source directory.")
        sys.exit(1)

    print("build linux")
    print("SYSROOT_DIR:%s" % sysroot_dir)

    # 运行 CMake 配置
    run(["cmake", "../../../",
         "-C", "../../../mk/android.cmake",
         "-DSOURCE_DIR=../../..",
         "-DCMAKE_TOOLCHAIN_FILE=%s" % os.path.join(ndk_root, "build/cmake/android.toolchain.cmake"),
         "-DANDROID_ABI=%s" % OPTION.arch,
         "-DANDROID_PLATFORM=android-21",
         "-DCMAKE_INSTALL_PREFIX=%s" % sysroot_dir],
        cwd=build_dir, check=True, err_msg="CMake configuration failed.")

    # 编译
    run(["make"], cwd=build_dir, check=True, err_msg="Make failed.")
    run(["make", "install"], cwd=build_dir, check=True, err_msg="Make install failed.")

    print("Build android completed.")


def ensure_windows_build_env():
    """普通 PowerShell 直接运行本脚本时补齐 MinGW / make 的 PATH。

    CMake 会用绝对路径找到 C:\\mingw64\\bin\\gcc.exe，但 gcc 启动时还要
    从 PATH 找 cc1.exe 和 MinGW 运行时 DLL；普通 PowerShell 的 PATH 里
    没有 C:\\mingw64\\bin，于是编译器测试会失败，且 gcc 往往不打印错误。
    """
    if platform.system() != "Windows":
        return

    # libobject 仍声明 cmake_minimum_required(VERSION 3.0)，新版 CMake
    # 会拒绝或警告；直接运行本脚本时也补上。
    os.environ.setdefault("CMAKE_POLICY_VERSION_MINIMUM", "3.5")

    candidates = [
        os.environ.get("MINGW_ROOT", ""),
        r"C:\mingw64",
        r"C:\msys64\mingw64",
        r"C:\msys64\ucrt64",
        r"C:\msys64\mingw32",
    ]
    mingw_bin = ""
    for root in candidates:
        if not root:
            continue
        bin_dir = os.path.join(root, "bin")
        if os.path.isfile(os.path.join(bin_dir, "gcc.exe")):
            mingw_bin = bin_dir
            break

    path_parts = [p for p in os.environ.get("PATH", "").split(os.pathsep) if p]
    normalized = [os.path.normcase(p) for p in path_parts]

    if mingw_bin and os.path.normcase(mingw_bin) not in normalized:
        os.environ["PATH"] = mingw_bin + os.pathsep + os.environ.get("PATH", "")
        print("Added to PATH: %s" % mingw_bin)

    # devops.py 里用 make 构建；普通 PowerShell 也没有 MSYS2 的 make。
    # 追加而不是前插，避免把 VS 自带的 cmake 换成 MSYS2 的 cmake。
    msys_bin = r"C:\msys64\usr\bin"
    if os.path.isfile(os.path.join(msys_bin, "make.exe")):
        path_parts = [p for p in os.environ.get("PATH", "").split(os.pathsep) if p]
        if os.path.normcase(msys_bin) not in [os.path.normcase(p) for p in path_parts]:
            os.environ["PATH"] = os.environ.get("PATH", "") + os.pathsep + msys_bin
            print("Added to PATH: %s" % msys_bin)


def do_build_windows():
    print("Building for Windows...")
    ensure_windows_build_env()
    increase_version_tweak_number()

    # 如果未指定架构，默认使用 x86_64
    if not OPTION.arch:
        print("No architecture specified. Defaulting to x86_64.")
        OPTION.arch = "x86_64"

    # 动态检测当前平台
    host = platform.system()
    if host == "Linux":
        toolchain_file = "../../../mk/windows.x86_64.cross.toolchain.cmake"
        print("Detected Linux host. Using cross-compilation toolchain: %s" % toolchain_file)
    elif host == "Windows":
        toolchain_file = "../../../mk/windows.x86_64.toolchain.cmake"
        print("Detected Windows host. Using native Windows toolchain: %s" % toolchain_file)
    else:
        print("Unsupported host platform: %s" % host)
        sys.exit(1)

    # 设置构建目录和安装目录
    os.makedirs(os.path.join(PROJECT_ROOT, "sysroot", "windows", OPTION.arch), exist_ok=True)
    build_dir = os.path.join(PROJECT_ROOT, "build", "windows", OPTION.arch)
    sysroot_dir = os.path.realpath(os.path.join(PROJECT_ROOT, "sysroot", "windows", OPTION.arch))

    # 创建构建目录
    os.makedirs(build_dir, exist_ok=True)

    # 检查工具链文件是否存在
    if not os.path.isfile(os.path.join(build_dir, toolchain_file)):
        print("Error: Toolchain file not found: %s" % toolchain_file)
        sys.exit(1)

    # 检查 CMakeLists.txt 文件是否存在于构建目录的父级
    if not os.path.isfile(os.path.join(build_dir, "../../../CMakeLists.txt")):
        print("Error: CMakeLists.txt not found in the source directory.")
        sys.exit(1)

    if host == "Linux":
        # 运行 CMake，加载工具链文件
        run(["cmake", "../../../", "-DPLATFORM=windows", "-DMODULE_UI=off",
             "-DCMAKE_TOOLCHAIN_FILE=%s" % toolchain_file,
             "-DCMAKE_INSTALL_PREFIX=%s" % sysroot_dir],
            cwd=build_dir, check=True, err_msg="CMake configuration failed.")
    else:
        print("build windows locally.")
        run(["cmake", "../../..", "-G", "Unix Makefiles", "-DPLATFORM=windows",
             "-DMODULE_UI=off",
             "-DCMAKE_TOOLCHAIN_FILE=%s" % toolchain_file,
             "-DCMAKE_INSTALL_PREFIX=%s" % sysroot_dir],
            cwd=build_dir, check=True, err_msg="CMake configuration failed.")

    run(["make"], cwd=build_dir, check=True, err_msg="Make failed.")

    if OPTION.install:
        run(["make", "install"], cwd=build_dir, check=True,
            err_msg="Make install failed.")
        sys.exit(0)

    copy_scripts_and_includes(build_dir, os.path.join(build_dir, sysroot_dir))


def do_build():
    if OPTION.platform == "linux":
        print("build linux")
        do_build_linux()
        sys.exit(0)
    elif OPTION.platform == "mac":
        print("build mac")
        print("PlatForm: %s" % OPTION.platform)
        do_build_mac()
    elif OPTION.platform == "ios":
        print("build ios")
        print("PlatForm: %s" % OPTION.platform)
        do_build_ios()
    elif OPTION.platform == "android":
        print("build android")
        print("PlatForm: %s" % OPTION.platform)
        do_build_android()
    elif OPTION.platform == "windows":
        print("-- build windows")
        print("-- PlatForm: %s" % OPTION.platform)
        do_build_windows()
    else:
        OPTION.help = True


def do_clean():
    print("clean build")
    shutil.rmtree(os.path.join(PROJECT_ROOT, "build"), ignore_errors=True)
    shutil.rmtree(os.path.join(PROJECT_ROOT, "sysroot"), ignore_errors=True)


def make_release_package(sysroot_dir, pkg_prefix):
    """release 公共流程：生成包名、检查重复、打包 sysroot 内容到 packages/"""
    version_file = os.path.join(PROJECT_ROOT, "src", "include", "libobject", "version.h")

    # 检查必要的文件和目录
    if not os.path.isdir(sysroot_dir):
        print("Error: %s does not exist." % sysroot_dir)
        sys.exit(1)
    if not os.path.isfile(version_file):
        print("Error: src/include/libobject/version.h does not exist.")
        sys.exit(1)

    # 获取发布包名称
    file_name = get_release_package_name(pkg_prefix, version_file)
    print("file_name: %s" % file_name)

    pkg_path = os.path.join(PROJECT_ROOT, "packages", file_name)
    # 如果包已存在，直接退出
    if os.path.isfile(pkg_path):
        print("Package %s already exists." % file_name)
        sys.exit(0)
    elif not file_name:
        print("Error: Failed to generate package name.")
        sys.exit(1)

    # 创建发布包，包内顶层目录为 xtools（等价于 bash 的 tar -zcvf $file_name xtools/*）
    packages_dir = os.path.join(PROJECT_ROOT, "packages")
    os.makedirs(packages_dir, exist_ok=True)
    with tarfile.open(pkg_path, "w:gz") as tar:
        for item in sorted(os.listdir(sysroot_dir)):
            tar.add(os.path.join(sysroot_dir, item),
                    arcname=os.path.join("xtools", item))

    print("Release package created: %s" % file_name)
    return 0


def do_release_linux():
    print("do release package for Linux")

    # 如果未指定架构，默认使用 x86_64
    if not OPTION.arch:
        print("No architecture specified. Defaulting to x86_64.")
        OPTION.arch = "x86_64"

    # 根据架构设置 SYSROOT_DIR
    if OPTION.arch == "arm":
        sysroot_dir = "sysroot/linux/arm"
    elif OPTION.arch in ("aarch64", "arm64"):
        sysroot_dir = "sysroot/linux/aarch64"
    else:
        sysroot_dir = "sysroot/linux/x86_64"

    make_release_package(os.path.join(PROJECT_ROOT, sysroot_dir),
                         "xtools_linux_%s" % OPTION.arch)


def do_release_windows():
    print("do release package for Windows")

    # 如果未指定架构，默认使用 x86_64
    if not OPTION.arch:
        print("No architecture specified. Defaulting to x86_64.")
        OPTION.arch = "x86_64"

    # 设置 SYSROOT_DIR
    sysroot_dir = "sysroot/windows/%s" % OPTION.arch

    make_release_package(os.path.join(PROJECT_ROOT, sysroot_dir),
                         "xtools_windows_%s" % OPTION.arch)


def do_release_android():
    print("do release package for Android")

    print("OPTION_ARCH: %s" % OPTION.arch)
    # 如果未指定架构，默认使用 arm64-v8a
    if not OPTION.arch:
        print("No architecture specified. Defaulting to arm64-v8a.")
        OPTION.arch = "arm64-v8a"
    print("OPTION_ARCH: %s" % OPTION.arch)

    # 设置 SYSROOT_DIR
    sysroot_dir = "sysroot/android/%s" % OPTION.arch

    make_release_package(os.path.join(PROJECT_ROOT, sysroot_dir),
                         "xtools_android_%s" % OPTION.arch)


def do_release():
    if OPTION.platform == "linux":
        do_release_linux()
    elif OPTION.platform == "windows":
        do_release_windows()
    elif OPTION.platform == "android":
        do_release_android()
    else:
        print("Unsupported platform: %s" % OPTION.platform)
        OPTION.help = True
        return 1
    return 0


def get_ssh_prefix():
    """密码自动应答前缀：优先 sshpass，否则裸调 ssh（交互输密码）"""
    if shutil.which("sshpass"):
        return ["sshpass", "-p", SSH_PASSWORD]
    return []


def do_deploy_linux():
    prefix = get_ssh_prefix()

    print("1.kill running xtools progess ...")
    ret = run(prefix + ["ssh", "root@%s" % OPTION.ip,
                        "ps -aux|grep \"xtools\" | grep -v grep | awk '{print $2}'"],
              capture=True)
    output = (ret.stdout or "").replace("\r", "")
    # 与 bash 版一致：跳过前 2 行（自动应答工具输出），取每行第 1 列
    pids = []
    for line in output.splitlines()[2:]:
        cols = line.split()
        if cols:
            pids.append(cols[0])
    print("ret:", " ".join(pids))

    for pid in pids:
        ret = run(prefix + ["ssh", "root@%s" % OPTION.ip, "kill -9", pid])
        if ret.returncode != 0:
            print("kill xtools process failed!")
        else:
            print("kill running xtools progess id: %s success" % pid)

    print("2.mkdir packages: %s" % OPTION.to_path)
    ret = run(prefix + ["ssh", "root@%s" % OPTION.ip, "mkdir -p %s" % OPTION.to_path])
    if ret.returncode != 0:
        print("uploading images failed!")
        sys.exit(1)

    if not os.path.isfile(OPTION.package_path):
        print("Error: package file not found: %s" % OPTION.package_path)
        sys.exit(1)

    print("3.copy package to %s:%s ..." % (OPTION.ip, OPTION.to_path))
    ret = run(prefix + ["scp", OPTION.package_path,
                        "root@%s:%s" % (OPTION.ip, OPTION.to_path)])
    if ret.returncode != 0:
        print("uploading images failed!")
        sys.exit(1)

    print("4.mkdir ~/.xtools/sysroot/")
    ret = run(prefix + ["ssh", "root@%s" % OPTION.ip,
                        "rm -rf ~/.xtools/sysroot/ && mkdir -p ~/.xtools/sysroot/"])
    if ret.returncode != 0:
        print("uploading images failed!")
        sys.exit(1)

    pkg_base = os.path.basename(OPTION.package_path)
    print("5.expand %s to working dir ..." % pkg_base)
    ret = run(prefix + ["ssh", "root@%s" % OPTION.ip,
                        "tar -zxvf %s/%s -C ~/.xtools/sysroot --strip-components 1"
                        % (OPTION.to_path, pkg_base)])
    if ret.returncode != 0:
        print("uploading images failed!")
        sys.exit(1)

    print("6.run xtools program")
    ret = run(prefix + ["ssh", "root@%s" % OPTION.ip,
                        "{ export LD_LIBRARY_PATH=~/.xtools/sysroot/lib:~/.xtools/sysroot/lib64:$LD_LIBRARY_PATH"
                        "&&nohup stdbuf -oL -eL ~/.xtools/sysroot/bin/xtools --log-level=0x20016 node "
                        "--host=0.0.0.0 --service=12345 --deamon=t; } >~/.xtools/logs 2>&1 &"])
    if ret.returncode != 0:
        print("run xtools program failed!")
        sys.exit(1)
    print("7.deploy xtools success!")


def do_deploy_android():
    print("Deploying to Android platform...")
    OPTION.to_path = "/data/local/tmp/.xtools/packages"
    sysroot_dir = "/data/local/tmp/.xtools/sysroot"

    print("1.mkdir packages: %s" % OPTION.to_path)
    ret = run(["adb", "shell", "mkdir -p %s" % OPTION.to_path])
    if ret.returncode != 0:
        print("Failed to create directory on Android device!")
        sys.exit(1)
    print("2.mkdir sysroot: %s" % sysroot_dir)
    ret = run(["adb", "shell", "mkdir -p %s" % sysroot_dir])
    if ret.returncode != 0:
        print("Failed to create directory on Android device!")
        sys.exit(1)

    pkg_base = os.path.basename(OPTION.package_path)
    ret = run(["adb", "shell", "test -f %s/%s" % (OPTION.to_path, pkg_base)])
    if ret.returncode != 0:
        print("3.copy package to Android device: %s ..." % OPTION.to_path)
        ret = run(["adb", "push", OPTION.package_path, OPTION.to_path])
        if ret.returncode != 0:
            print("Failed to copy package to Android device!")
            sys.exit(1)
        print("4.expand %s to working dir ..." % pkg_base)
        ret = run(["adb", "shell",
                   "tar -zxvf %s/%s -C %s --strip-components 1"
                   % (OPTION.to_path, pkg_base, sysroot_dir)])
        if ret.returncode != 0:
            print("Failed to extract package on Android device!")
            sys.exit(1)

    print("5.run xtools program")
    ret = run(["adb", "shell",
               "export LD_LIBRARY_PATH=%s/lib:$LD_LIBRARY_PATH && nohup stdbuf -oL -eL "
               "%s/bin/xtools --log-level=0x20016 node --host=139.159.231.27 "
               "--service=12345 >%s/logs 2>&1 &" % (sysroot_dir, sysroot_dir, sysroot_dir)])
    if ret.returncode != 0:
        print("Failed to run xtools program on Android device!")
        sys.exit(1)

    print("6.deploy xtools success on Android!")


def do_deploy():
    print("do deploy!")
    if OPTION.platform == "linux":
        print("deploy xtools on linux!")
        do_deploy_linux()
    elif OPTION.platform == "android":
        print("deploy xtools on Android!")
        do_deploy_android()
    else:
        print("Unsupported platform for deployment: %s" % OPTION.platform)
        OPTION.help = True
        return 1
    return 0


def do_install_docker():
    if not OPTION.platform:
        print("\nplease set platform when install docker\n")
        sys.exit(1)
    print("do_install_docker")
    if platform.system() == "Linux":
        ret = run(["lsb_release", "--short", "--id"], capture=True)
        distro = (ret.stdout or "").strip()
        ret = run(["lsb_release", "--short", "--release"], capture=True)
        distro_version = (ret.stdout or "").strip()
        print("%s %s" % (distro, distro_version))
    if OPTION.platform == "linux":
        if platform.system() != "Linux":
            print("install docker only supported on linux host")
            sys.exit(1)
        print("install docker at linux")
        if not shutil.which("curl"):
            print("install curl, please wait...")
            run(["sudo", "apt-get", "install", "curl"])
        if not shutil.which("docker"):
            print("install docker, please wait...")
            run("curl -fsSL https://get.docker.com | bash -s docker --mirror Aliyun")
        ret = run(["who"], capture=True)
        who_out = (ret.stdout or "").split()
        user_name = who_out[0] if who_out else os.environ.get("USER", "")
        print("user_name:%s" % user_name)
        run(["sudo", "usermod", "-aG", "docker", user_name])
        sys.exit(0)
    elif OPTION.platform in ("mac", "ios", "android"):
        sys.exit(0)


def do_build_docker_image_xtools():
    print("do_build_docker_image_xtools, pwd:%s" % os.getcwd())
    version = DOCKER_IMAGE_VERSION

    # 确定 Dockerfile 路径
    if not OPTION.dockerfile:
        print("Error: --dockerfile parameter is required. Please specify Dockerfile name.")
        print("Example: --dockerfile=dockerfile.noded or --dockerfile=dockerfile.test")
        sys.exit(1)

    # 使用指定的 Dockerfile
    dockerfile_path = "docker/%s" % OPTION.dockerfile
    print("Using Dockerfile: %s" % dockerfile_path)

    # 检查 Dockerfile 是否存在
    if not os.path.isfile(os.path.join(PROJECT_ROOT, dockerfile_path)):
        print("Error: Dockerfile not found at %s" % dockerfile_path)
        sys.exit(1)

    run(["docker", "build", "-f", dockerfile_path,
         "-t", OPTION.name,
         "-t", "%s:%s" % (OPTION.name, version),
         "--build-arg", "TAR_PACKAGE_PATH=%s" % OPTION.package_path,
         "."], cwd=PROJECT_ROOT)


def do_remove_docker_container():
    if not OPTION.name:
        OPTION.help = True
        return 1
    ret = run(["docker", "ps", "-a"], capture=True)
    container_id = ""
    for line in (ret.stdout or "").splitlines():
        if OPTION.name in line:
            container_id = line.split()[0]
            break
    print("remover docker container, container id:%s" % container_id)
    run(["docker", "stop", container_id])
    run(["docker", "rm", container_id])
    return 0


def do_run_docker_container():
    if not OPTION.name:
        OPTION.help = True
        return 1
    ret = run(["docker", "images"], capture=True)
    image_name = ""
    for line in (ret.stdout or "").splitlines():
        if OPTION.name in line:
            image_name = line.split()[0]
            break
    print("run docker container, container image_name:%s OPTION_ARGS:%s"
          % (image_name, OPTION.args))
    cmd = ["docker", "run", "-itd"]
    if OPTION.args:
        cmd += OPTION.args.split()
    cmd.append(OPTION.name)
    run(cmd)
    ret = run(["docker", "ps"], capture=True)
    container_id = ""
    for line in (ret.stdout or "").splitlines():
        if OPTION.name in line:
            container_id = line.split()[0]
            break
    run(["docker", "exec", "-it", container_id, "/bin/bash"])
    return 0


def do_build_docker_image():
    print("do_build_docker_image image name:%s" % OPTION.name)

    if not OPTION.name:
        OPTION.help = True
        return 1
    do_build_docker_image_xtools()
    return 0


def resolve_test_platforms():
    """把 --platform 解析成待测平台列表；`all`/空 表示 linux + windows"""
    p = (OPTION.platform or "all").strip().lower()
    if p in ("", "all"):
        return ["linux", "windows"]
    if p in ("linux", "windows"):
        return [p]
    print("Error: unsupported platform for test: %s (supported: linux, windows, all)"
          % OPTION.platform)
    sys.exit(1)


def test_arch_for(platform_name):
    """linux 测试支持 x86_64/arm/aarch64；windows 只有 x86_64"""
    arch = OPTION.arch or "x86_64"
    if platform_name == "linux":
        if arch == "arm64":
            arch = "aarch64"
        if arch not in ("x86_64", "arm", "aarch64"):
            print("Error: unsupported linux arch for test: %s "
                  "(supported: x86_64, arm, aarch64)" % arch)
            sys.exit(1)
        return arch
    # windows 仅有 x86_64
    return "x86_64"


def test_xtools_path(platform_name, arch):
    exe = "xtools.exe" if platform_name == "windows" else "xtools"
    return os.path.join(PROJECT_ROOT, "sysroot", platform_name, arch, "bin", exe)


def _mockery_cmd(xtools, filter_name):
    return [xtools,
            "--log-type=%s" % OPTION.log_type,
            "--log-level=%s" % OPTION.log_level,
            "mockery", "-f", filter_name]


def run_mockery_native(platform_name, arch):
    host = platform.system()
    xtools = test_xtools_path(platform_name, arch)
    if not os.path.isfile(xtools):
        print("Error: %s not found. Build first with:" % xtools)
        print("  python devops.py build --platform=%s --arch=%s" % (platform_name, arch))
        return 1
    print("run tests natively on %s: %s" % (host, xtools))
    ret = run(_mockery_cmd(xtools, OPTION.filter), cwd=PROJECT_ROOT)
    return ret.returncode


def run_mockery_linux_in_docker(arch):
    ensure_dev_image()
    # Windows 上 docker -v 的盘符冒号可能被误解析，统一转正斜杠
    host_dir = PROJECT_ROOT.replace(os.sep, "/")
    inner = ("cd /work && ./sysroot/linux/%s/bin/xtools "
             "--log-type=%s --log-level=%s mockery -f %s"
             % (arch, OPTION.log_type, OPTION.log_level, OPTION.filter))
    cmd = ["docker", "run", "--rm",
           "-v", "%s:/work" % host_dir,
           DOCKER_DEV_IMAGE, "bash", "-lc", inner]
    print("run linux tests inside docker container %s (host=%s)"
          % (DOCKER_DEV_IMAGE, platform.system()))
    ret = run(cmd, cwd=PROJECT_ROOT)
    return ret.returncode


def do_test_platform(platform_name):
    """跑一个平台的测试；返回退出码，None 表示该平台在当前 host 上无法测试被跳过"""
    host = platform.system()
    arch = test_arch_for(platform_name)

    can_run_native = (host == "Linux") if platform_name == "linux" else (host == "Windows")
    if can_run_native:
        return run_mockery_native(platform_name, arch)

    # 非本机平台：目前只有 linux 能通过 docker 在非 Linux host 上测
    if platform_name == "linux":
        return run_mockery_linux_in_docker(arch)

    print("Skip %s: cannot run %s tests on host %s "
          "(no supported test backend)" % (platform_name, platform_name, host))
    return None


def do_test():
    print("do_test")
    platforms = resolve_test_platforms()
    failed = []
    for p in platforms:
        print("=" * 70)
        rc = do_test_platform(p)
        if rc is None:
            continue
        if rc != 0:
            failed.append(p)
    print("=" * 70)
    if failed:
        print("test FAILED for: %s" % ", ".join(failed))
        sys.exit(1)
    print("all requested tests passed.")


def do_help():
    print("""    Usage:
    devops.py build|clean|release|deploy|docker|test|help 
            [-p|--platform=<OPTION_PLATFORM>] [-i|--install] [--install-docker] 
            [--db-ip=IP_ADRESS] [--package-path]
            [--build=PACKAGE_NAME] [--run=PACKAGE_NAME] [--remove=PACKAGE_NAME]
    
    commands:
    build                  build this repo.
        -p, --platform:    Optional, OPTION_PLATFORM can be: linux, mac, windows, ios, android.
        --install:         Optional. install lib, bin, and include files.
        --docker:          Optional. build inside the dev container (docker/dockerfile.dev).

    release                release package
        -p, --platform:    Optional, OPTION_PLATFORM can be: linux, mac, windows, ios, android.
        --docker:          Optional. release inside the dev container (docker/dockerfile.dev).

    clean                  clean build files.  

    shell                  enter an interactive shell in the dev container
                           (docker/dockerfile.dev, source mounted at /work).

    docker                 do docker relevant operations.
        --db-ip            Optional. db ip address.
        --package-root     Optional. deploy package root path.
        --install          Optional. install docker
        --remove           Optional. remove container
        --run              Optional. run container
        --args             Optional. designate args.
        --build            build docker.
        --dockerfile       Required. specify Dockerfile name (e.g., --dockerfile=dockerfile.noded uses docker/dockerfile.noded)

    test                   run mockery tests in the built binaries.
        -p, --platform:    linux, windows or all (default all). On a Windows host,
                           linux tests run inside the dev container via docker.
        --arch:            linux arch: x86_64 (default), arm, aarch64. windows is always x86_64.
        -f, --filter:      mockery -f <name> (prefix match), default all.
        --log-level:       xtools log level, default 0x16.
        --log-type:        xtools log output type: 0=console, 1=file, 2=network, default 0.

    help                   Print this help message.

    demos:
    python devops.py build --platform=linux
    python devops.py build --platform=linux --docker        #build inside dev container
    python devops.py release --platform=linux --docker      #release inside dev container
    python devops.py shell                                  #enter dev container shell
    python devops.py test --platform=linux                  #linux tests (docker on Windows host)
    python devops.py test --platform=windows                #windows tests (native)
    python devops.py test                                   #linux + windows
    python devops.py test -p linux -f test_archive_         #only a subset of tests
    python devops.py test -p linux --log-level=0x1ffff      #verbose logs
    python devops.py build --platform=windows
    python devops.py docker --install --platform=linux    #install docker at linux platform
    python devops.py release -p=linux
    python devops.py deploy -p=linux --host=139.159.231.27 --package-path=./packages/xtools_v2.13.0.442.tar.gz
    python devops.py docker --build=fruit-noded --package-path=./packages/xtools_linux_x86_64_v2.15.0.229.tar.gz
    
""")


def parse_args(argv):
    """解析命令行参数，兼容 bash 版 --opt=value 与 -p=value 形式"""
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("command", nargs="?", default="")
    parser.add_argument("-p", "--platform", dest="platform", default="")
    parser.add_argument("-i", "--install", action="store_true", default=False)
    parser.add_argument("--arch", default="")
    parser.add_argument("-f", "--filter", dest="filter", default="all")
    parser.add_argument("--log-type", dest="log_type", default="0")
    parser.add_argument("--log-level", dest="log_level", default="0x16")
    parser.add_argument("--build", dest="build_image", default="")
    parser.add_argument("--remove", dest="remove_name", default="")
    parser.add_argument("--run", dest="run_name", default="")
    parser.add_argument("--args", dest="args", default="")
    parser.add_argument("--host", dest="host", default="")
    parser.add_argument("--to", dest="to", default="")
    parser.add_argument("--package-path", dest="package_path", default="")
    parser.add_argument("--dockerfile", dest="dockerfile", default="")
    parser.add_argument("--docker", action="store_true", default=False)
    # 未知参数与 bash 版行为一致：打印帮助并退出 0
    args, unknown = parser.parse_known_args(argv)
    if unknown:
        print("Unknown option %s" % " ".join(unknown))
        OPTION.help = True

    if args.command in ("build", "clean", "release", "deploy", "docker",
                        "shell", "test", "help"):
        OPTION.cmd_name = args.command
    elif args.command:
        print("Unknown option %s" % args.command)
        OPTION.help = True

    OPTION.platform = args.platform
    OPTION.install = args.install
    OPTION.arch = args.arch
    OPTION.filter = args.filter
    OPTION.log_type = args.log_type
    OPTION.log_level = args.log_level
    OPTION.args = args.args
    OPTION.ip = args.host
    OPTION.package_path = args.package_path
    OPTION.dockerfile = args.dockerfile
    OPTION.docker = args.docker
    # 透传给容器内 devops.py 的参数（去掉 --docker，防止容器内递归）
    OPTION.forward_args = [a for a in argv if a != "--docker"]
    if args.to:
        OPTION.to_path = args.to
    if args.build_image:
        OPTION.build = True
        OPTION.name = args.build_image
    if args.remove_name:
        OPTION.remove = True
        OPTION.name = args.remove_name
    if args.run_name:
        OPTION.run = True
        OPTION.name = args.run_name


def ensure_dev_image():
    """确保开发镜像存在，不存在则用 docker/dockerfile.dev 构建"""
    if not shutil.which("docker"):
        print("Error: docker not found. Please install docker first.")
        sys.exit(1)
    ret = run(["docker", "images", "-q", DOCKER_DEV_IMAGE], capture=True)
    if not (ret.stdout or "").strip():
        print("dev image %s not found, building from %s ..."
              % (DOCKER_DEV_IMAGE, DOCKER_DEV_DOCKERFILE))
        run(["docker", "build", "-f", DOCKER_DEV_DOCKERFILE,
             "-t", DOCKER_DEV_IMAGE, "."],
            cwd=PROJECT_ROOT, check=True, err_msg="Docker image build failed.")


def run_in_docker():
    """在开发容器（docker/dockerfile.dev 构建的镜像）内执行 build/release"""
    ensure_dev_image()

    # 工程根目录挂载到容器 /work，透传原始参数（已去掉 --docker）
    cmd = ["docker", "run", "--rm",
           "-v", "%s:/work" % PROJECT_ROOT,
           DOCKER_DEV_IMAGE, "python3", "devops.py"] + OPTION.forward_args
    ret = run(cmd)
    sys.exit(ret.returncode)


def do_docker_shell():
    """进入开发容器的交互 shell（工程根目录挂载到 /work）"""
    ensure_dev_image()
    # 非 tty 环境（如管道）下去掉 -t，便于脚本调用
    tty_args = ["-it"] if sys.stdin.isatty() else ["-i"]
    cmd = (["docker", "run", "--rm"] + tty_args +
           ["-v", "%s:/work" % PROJECT_ROOT, DOCKER_DEV_IMAGE, "bash"])
    ret = run(cmd)
    sys.exit(ret.returncode)


def process_args():
    if OPTION.cmd_name == "build":
        if OPTION.docker:
            run_in_docker()
        else:
            do_build()
    elif OPTION.cmd_name == "release":
        if OPTION.docker:
            run_in_docker()
        else:
            do_release()
    elif OPTION.cmd_name == "shell":
        do_docker_shell()
    elif OPTION.cmd_name == "clean":
        do_clean()
        sys.exit(0)
    elif OPTION.cmd_name == "docker":
        print("docker")
        if OPTION.install:
            print("install docker")
            do_install_docker()
            sys.exit(0)
        elif OPTION.build:
            print("build")
            do_build_docker_image()
        elif OPTION.remove:
            do_remove_docker_container()
        elif OPTION.run:
            do_run_docker_container()
        else:
            OPTION.help = True
    elif OPTION.cmd_name == "deploy":
        do_deploy()
        sys.exit(0)
    elif OPTION.cmd_name == "test":
        do_test()
    elif OPTION.cmd_name == "help":
        do_help()
        sys.exit(0)
    else:
        OPTION.help = True

    if OPTION.help:
        do_help()
        sys.exit(0)


def main():
    # 与 bash 版一致，默认在工程根目录下执行
    os.chdir(PROJECT_ROOT)
    parse_args(sys.argv[1:])
    process_args()


if __name__ == "__main__":
    main()
