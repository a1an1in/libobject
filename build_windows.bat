
mkdir -p build/windows
cd build/windows
cmake.exe ../.. -DPLATFORM=windows && mingw32-make.exe
