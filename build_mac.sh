#! /bin/bash
rm -rf /usr/local/include/libobject
mkdir -p build/mac
cd build/mac
cmake ../.. -DPLATFORM=mac &&make
#make install
cd ..
