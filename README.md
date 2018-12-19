# libobject

[TOC]

## Intro
This lib is designed for object-oriented programming using c language. It contains core, event, net, concurrent, and ui (not completed) modules now. Every module supply demos which are easy.

## Installation

### Mac OS

1. cmake .

   change diretory to this lib, and key 'cmake .' command.
   if you want to config modules to compile, there're two methods recommended.

   first, pass module switch to cmake like below.

   ```
   cmake . -DMODULE_UI=ON
   ```

   sencond, using ccmake command to config.

2. make 

   and key 'make' command.

3. make install

   and key 'make install' command.

### Linux

​	refer to Mac OS method.

### Android

1. Install android ndk

2. set ndk-root evironment variable.

3. Begin the specified project compilation.

   cd xxxx      //cd to project root

   mkdir -p build/android

   cmake  -DPlatform=android -DCMAKE_TOOLCHAIN_FILE=$NDK_ROOT/build/cmake/android.toolchain.cmake ../../

### Windows

​	Sorry, I hav't test using Cmake on this platform.

## User Guide

## Contact
The latest version of the lib is available at https://github.com/a1an1in/libobject. if you have some prolems about this lib you could also contact me with email a1an1in@sina.com.

## License
This software is licensed under the LGPL license. Copyright (c) 2015-2020.
