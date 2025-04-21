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

./devops.sh build --platform=android --arch=arm64-v8a
./devops.sh release --platform=android --arch=arm64-v8a
./devops.sh deploy -p=android --package-path=./packages/xtools_android_arm64-v8a_v2.15.0.154.tar.gz
adb shell 
cd  /data/local/tmp/.xtools/
export LD_LIBRARY_PATH=/data/local/tmp/.xtools/sysroot/lib:$LD_LIBRARY_PATH
./sysroot/bin/xtools --log-level=0x20017 node -h
./sysroot/bin/xtools node --log-level=0x15 --host=139.159.231.27 --service=12345
nohup ./sysroot/bin/xtools node --log-level=0x15 --host=139.159.231.27 --service=12345 >/data/local/tmp/.xtools/logs 2>&1 &
./sysroot/linux/x86_64/bin/xtools node_cli --host="139.159.231.27" --service="12345" lookup all 
```
