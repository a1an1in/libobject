#! /bin/bash
#cmake -DPLATFORM=android -DANDROID_ABI=x86_64&&make&&
#rm -rf CMakeFiles&&rm CMakeCache.txt&&
#cmake -DPLATFORM=android -DANDROID_ABI=x86&&make&&
#rm -rf CMakeFiles&&rm CMakeCache.txt&&
#cmake -DPLATFORM=android -DANDROID_ABI=arm64-v8a&&make&&
#rm -rf CMakeFiles&&rm CMakeCache.txt&&
rm -rf CMakeFiles&&rm CMakeCache.txt&&
cmake -DPLATFORM=android -DANDROID_ABI=armeabi-v7a&&make&&
cp -rf lib/android/ ~/workspace/goya/android/goya/app/src/main/jni/lib
cp lib/android/armeabi-v7a/*.so /Users/alanlin/Library/Android/sdk/ndk-bundle/platforms/android-21/arch-arm/usr/lib
rm -rf /Users/alanlin/Library/Android/sdk/ndk-bundle/sysroot/usr/include/libobject
cp -rf src/include/* /Users/alanlin/Library/Android/sdk/ndk-bundle/sysroot/usr/include/
