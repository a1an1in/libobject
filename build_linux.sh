#! /bin/bash
rm -rf CMakeFiles  CMakeCache.txt    cmake_install.cmake 
cmake . -DPLATFORM=linux -DMODULE_UI=on
make
make install
