#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""向 test-process 进程发送 30 号信号（等价 scripts/test.sh）"""

import os
import platform
import subprocess
import sys


def find_test_process_pids():
    """解析 ps -aux 输出，查找 test-process 进程 pid"""
    ret = subprocess.run(["ps", "-aux"], stdout=subprocess.PIPE,
                         stderr=subprocess.DEVNULL, text=True)
    pids = []
    for line in ret.stdout.splitlines():
        if "test-process" in line and "grep" not in line:
            cols = line.split()
            if len(cols) >= 2:
                pids.append(cols[1])
    return pids


def main():
    if platform.system() == "Windows":
        print("Windows 平台不支持该操作")
        sys.exit(1)

    pids = find_test_process_pids()
    if not pids:
        print("test-process not found")
        sys.exit(1)

    for pid in pids:
        os.kill(int(pid), 30)


if __name__ == "__main__":
    main()
