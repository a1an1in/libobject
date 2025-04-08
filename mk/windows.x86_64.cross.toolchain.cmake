# 定义交叉编译标记
add_definitions(-DCROSS_COMPILE)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10)

# 指定交叉编译器
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# 指定目标架构
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# 配置链接目录
link_directories(
    /usr/x86_64-w64-mingw32/lib
    /usr/lib/gcc/x86_64-w64-mingw32/posix
    ${CMAKE_INSTALL_PREFIX}/lib
)

# 配置包含目录
include_directories(
    /usr/x86_64-w64-mingw32/include
    /usr/lib/gcc/x86_64-w64-mingw32/posix/include
    ${PROJECT_SOURCE_DIR}/src/include
    ${CMAKE_INSTALL_PREFIX}/include
)