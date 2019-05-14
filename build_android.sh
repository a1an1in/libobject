#! /bin/bash
#cmake -DPLATFORM=android -DANDROID_ABI=x86_64&&make&&
#rm -rf CMakeFiles&&rm CMakeCache.txt&&
#cmake -DPLATFORM=android -DANDROID_ABI=x86&&make&&
#rm -rf CMakeFiles&&rm CMakeCache.txt&&
#cmake -DPLATFORM=android -DANDROID_ABI=arm64-v8a&&make&&
#rm -rf CMakeFiles&&rm CMakeCache.txt&&

rm -rf ${NDK_ROOT}/sysroot/usr/include/libobject

mkdir -p build/android
cd build/android
cmake ../.. -DPLATFORM=android -DANDROID_ABI=armeabi-v7a &&make&&make install
#cmake ../.. -DPLATFORM=android -DANDROID_ABI=x86 &&make&&make install
#cmake ../.. -DPLATFORM=android -DANDROID_ABI=x86_64 &&make&&make install
cd ../..

