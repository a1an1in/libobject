mkdir -p build\windows
cd /d build\windows
cmake.exe -S ..\.. -DPLATFORM=windows  -G "MinGW Makefiles"
mingw32-make.exe
cd ../..