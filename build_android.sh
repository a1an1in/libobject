#! /bin/bash
cmake -DPLATFORM=android -DANDROID_ABI=x86_64&&make&&
rm -rf CMakeFiles&&rm CMakeCache.txt&&
cmake -DPLATFORM=android -DANDROID_ABI=x86&&make&&
rm -rf CMakeFiles&&rm CMakeCache.txt&&
cmake -DPLATFORM=android -DANDROID_ABI=arm64-v8a&&make&&
rm -rf CMakeFiles&&rm CMakeCache.txt&&
cmake -DPLATFORM=android -DANDROID_ABI=armeabi-v7a&&make&&
rm -rf CMakeFiles&&rm CMakeCache.txt&&
cp -rf lib/android/ ~/workspace/goya/android/JniTest/app/libs/
