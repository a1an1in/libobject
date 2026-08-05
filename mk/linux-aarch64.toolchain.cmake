# 设置目标系统名称和处理器架构
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR aarch64)

# 指定交叉编译工具链（aarch64 工具链）
SET(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)
SET(CMAKE_LINKER /usr/bin/aarch64-linux-gnu-ld)
SET(CMAKE_AR /usr/bin/aarch64-linux-gnu-ar)

# 指定目标系统的根路径（sysroot）
SET(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)

# 配置搜索路径模式
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 设置编译器选项
SET(CMAKE_C_FLAGS "")
SET(CMAKE_CXX_FLAGS "")

# 设置 aarch64 平台的输出路径
SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/linux/aarch64)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)

# 指定库路径
LINK_DIRECTORIES(
    /usr/aarch64-linux-gnu/lib
    /usr/local/lib
)

# 指定头文件路径
INCLUDE_DIRECTORIES(
    /usr/aarch64-linux-gnu/include
    /usr/local/include
    ${PROJECT_SOURCE_DIR}/src/include
)
