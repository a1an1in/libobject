#! /bin/bash
rm -rf /usr/local/include/libobject&&
rm -rf CMakeFiles&&rm CMakeCache.txt&&cmake -DPLATFORM=mac -DMODULE_UI=on&&make&&make install
