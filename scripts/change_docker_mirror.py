#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""配置 docker 镜像加速器（等价 scripts/change_docker_mirror.sh）"""

import platform
import subprocess
import sys

DAEMON_JSON = """{
  "registry-mirrors": ["https://yxzrazem.mirror.aliyuncs.com"]
}
"""


def run(cmd):
    print("+ %s" % " ".join(cmd))
    ret = subprocess.run(cmd)
    if ret.returncode != 0:
        sys.exit(ret.returncode)


def main():
    if platform.system() != "Linux":
        print("非 Linux 平台不支持该操作")
        sys.exit(1)

    run(["sudo", "mkdir", "-p", "/etc/docker"])
    # 等价 sudo tee /etc/docker/daemon.json
    ret = subprocess.run(["sudo", "tee", "/etc/docker/daemon.json"],
                         input=DAEMON_JSON, text=True)
    if ret.returncode != 0:
        sys.exit(ret.returncode)
    run(["sudo", "systemctl", "daemon-reload"])
    run(["sudo", "systemctl", "restart", "docker"])


if __name__ == "__main__":
    main()
