#! /bin/bash
rm -rf /usr/local/include/libobject
mkdir -p build/mac
cd build/mac
cmake ../.. -DPLATFORM=mac &&make
#make install
cd ../..
printf '\x07' | dd of=sysroot/mac/bin/xtools bs=1 seek=160 count=1 conv=notrunc
