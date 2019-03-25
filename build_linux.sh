#! /bin/bash
rm -rf /usr/local/include/libobject
mkdir -p build/mac
cd build/mac
cmake ../.. -DPLATFORM=linux -DMODULE_UI=off
make
