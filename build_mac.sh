#! /bin/bash
rm -rf /usr/local/include/libobject
mkdir -p build/mac
cd build/mac
cmake ../.. -DPLATFORM=mac -DMODULE_UI=on&&make&&make install
cd ..
