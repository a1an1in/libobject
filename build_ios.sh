#! /bin/bash
mkdir -p build/ios
cd build/ios
#cmake ../.. -DPLATFORM=ios -DIOS_PLATFORM=SIMULATOR64 -DCMAKE_TOOLCHAIN_FILE=../../mk/ios.toolchain.cmake
cmake ../.. -DPLATFORM=ios  -DIOS_PLATFORM=OS -DCMAKE_TOOLCHAIN_FILE=../../mk/ios.toolchain.cmake
#cmake ../.. -DPLATFORM=ios  -DIOS_ARCH=armv7 -DCMAKE_TOOLCHAIN_FILE=../../mk/ios.toolchain.cmake
make
#make install
cd ../..
