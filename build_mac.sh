#! /bin/bash
rm -rf CMakeFiles&&rm CMakeCache.txt&&cmake -DPLATFORM=mac -DMODULE_UI=on&&make
