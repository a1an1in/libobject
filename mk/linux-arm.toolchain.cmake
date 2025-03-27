# 设置目标系统名称和处理器架构
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR arm)

# 指定交叉编译工具链（硬浮点工具链）
SET(CMAKE_C_COMPILER /usr/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_LINKER /usr/bin/arm-linux-gnueabihf-ld)
SET(CMAKE_AR /usr/bin/arm-linux-gnueabihf-ar)

# 指定目标系统的根路径（sysroot）
SET(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)

# 配置搜索路径模式
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 设置编译器选项（硬浮点配置）
SET(CMAKE_C_FLAGS "-march=armv7-a -mfpu=neon -mfloat-abi=hard")
SET(CMAKE_CXX_FLAGS "-march=armv7-a -mfpu=neon -mfloat-abi=hard")

# 设置 ARM 平台的输出路径
SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/linux/arm)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)

# 指定库路径
LINK_DIRECTORIES(
    /usr/arm-linux-gnueabihf/lib
    /usr/local/lib
)

# 指定头文件路径
INCLUDE_DIRECTORIES(
    /usr/arm-linux-gnueabihf/include
    /usr/local/include
    ${PROJECT_SOURCE_DIR}/src/include
)