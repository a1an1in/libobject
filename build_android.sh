#! /bin/bash
#cmake -DPLATFORM=android -DANDROID_ABI=x86_64&&make&&
#rm -rf CMakeFiles&&rm CMakeCache.txt&&
#cmake -DPLATFORM=android -DANDROID_ABI=x86&&make&&
#rm -rf CMakeFiles&&rm CMakeCache.txt&&
#cmake -DPLATFORM=android -DANDROID_ABI=arm64-v8a&&make&&
#rm -rf CMakeFiles&&rm CMakeCache.txt&&
rm -rf CMakeFiles&&rm CMakeCache.txt&&
rm -rf ${NDK_ROOT}/sysroot/usr/include/libobject&&
rm -rf ~/workspace/goya-github/android/goya-alone/app/src/main/jni/include/libobject

cmake -DPLATFORM=android -DANDROID_ABI=armeabi-v7a&&make&&

cp -rf lib/android/ ~/workspace/goya-github/android/goya/app/src/main/jni/lib
cp -rf lib/android/ ~/workspace/goya-github/android/goya-alone/app/src/main/jni/lib
cp lib/android/armeabi-v7a/*.so ${NDK_ROOT}/platforms/android-21/arch-arm/usr/lib

cp -rf src/include/libobject ${NDK_ROOT}/sysroot/usr/include/
cp -rf src/include/libobject ~/workspace/goya-github/android/goya-alone/app/src/main/jni/include/libobject
