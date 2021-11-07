#! /bin/bash
rm -rf /usr/local/include/libobject
mkdir -p build/linux
cd build/linux
cmake ../.. -DPLATFORM=linux -DMODULE_UI=off
make
