#! /bin/bash
mkdir -p build/ios
cd build/ios
cmake ../.. -DPLATFORM=ios -DIOS_PLATFORM=SIMULATOR64 -DCMAKE_TOOLCHAIN_FILE=../../mk/ios.toolchain.cmake -DMODULE_UI=on
#cmake ../.. -DPLATFORM=ios  -DIOS_PLATFORM=OS -DCMAKE_TOOLCHAIN_FILE=../../mk/ios.toolchain.cmake
make
make install
cd ../..
