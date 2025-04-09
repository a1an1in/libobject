1. 确保adb能正常链接。
2. 查看ABI版本
```bash
adb shell getprop ro.product.cpu.abi 
```
若输出结果为 arm64-v8a，则表明设备架构为ARM64‌
3. 部署脚本
```bash
./devops.sh build --platform=android --arch=arm64-v8a
./devops.sh release --platform=android --arch=arm64-v8a
./devops.sh deploy -p=android --package-path=./packages/xtools_android_arm64-v8a_v2.15.0.130.tar.gz

adb shell "mkdir -p /data/local/tmp/.xtools/packages && chmod 777 /data/local/tmp/.xtools"
adb push ./packages/xtools_android_arm64-v8a_v2.15.0.128.tar.gz /data/local/tmp/.xtools/packages
adb shell "mkdir -p /data/local/tmp/.xtools/sysroot"
adb shell "tar -zxvf /data/local/tmp/.xtools/packages/xtools_android_arm64-v8a_v2.15.0.128.tar.gz -C sdcard/.xtools/sysroot --strip-components 1"

adb shell "export LD_LIBRARY_PATH=/data/local/tmp/.xtools/sysroot/lib:$LD_LIBRARY_PATH&&/data/local/tmp/.xtools/sysroot/bin/xtools -h"
adb shell "export LD_LIBRARY_PATH=/data/local/tmp/.xtools/sysroot/lib:$LD_LIBRARY_PATH&&/data/local/tmp/.xtools/sysroot/bin/xtools --log-level=0x20016 node -h"

adb shell ls -la /data/local/tmp/.xtools/ -h
adb shell cat /data/local/tmp/.xtools/dbg.ini -h
```
