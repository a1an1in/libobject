#! /bin/bash
mkdir -p build/android
cd build/android
cmake ../.. -DPLATFORM=android -DANDROID_ABI=armeabi-v7a -DCMAKE_ANDROID_NDK=/Users/alanlin/Library/Android/sdk/ndk-bundle&&make&&make install
cd ../..

