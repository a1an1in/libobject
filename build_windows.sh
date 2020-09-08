#! /bin/bash
rm -rf /usr/local/include/libobject
mkdir -p build/windows
cd build/windows
cmake ../.. -DPLATFORM=windows &&make
#make install
