#! /bin/bash
rm /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/lib/libobject.*
rm -rf /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/include/libobject
mkdir -p build/ios
cd build/ios
cmake ../.. -DPLATFORM=ios -DIOS_PLATFORM=SIMULATOR64 -DCMAKE_TOOLCHAIN_FILE=../../mk/ios.toolchain.cmake -DMODULE_UI=on
make

make install
cd ../..
#cp -rf src/include/libobject /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/include/
