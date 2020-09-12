mkdir -p build\windows
cd /d build\windows
cmake.exe -S ..\.. -DPLATFORM=windows  -G "MinGW Makefiles" -DCMAKE_MAKE=mingw32-make
mingw32-make.exe
