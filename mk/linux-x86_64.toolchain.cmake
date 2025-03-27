# 设置目标系统名称和处理器架构
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR x86_64)

# 指定编译器
SET(CMAKE_C_COMPILER /usr/bin/gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/g++)

# 指定库路径
LINK_DIRECTORIES(
    /usr/lib/x86_64-linux-gnu
    /usr/local/lib
)

# 指定头文件路径
INCLUDE_DIRECTORIES(
    /usr/include
    /usr/include/mysql
    /usr/include/python3.8
    /usr/local/include
    ${PROJECT_SOURCE_DIR}/src/include
)

# 设置 x86_64 平台的输出路径
SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/linux/x86_64)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)