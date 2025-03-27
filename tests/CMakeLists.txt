project(object-tests)

# 获取所有头文件
file(GLOB_RECURSE MOUDLE_HEADERS
    "*.h"
)

# 获取所有源文件
file(GLOB_RECURSE MOUDLE_SOURCES
    "*.c"
)

# 检测是否为 ARM 平台
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    message(STATUS "ARM platform detected. Skipping tests/attacher and tests/database sources.")
    # 从 MOUDLE_SOURCES 中移除 tests/attacher 和 tests/database 目录的文件
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/attacher/.*")
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/database/.*")
endif()

# 将头文件和源文件合并
file(GLOB MOUDLE_SRC
    ${MOUDLE_HEADERS}
    ${MOUDLE_SOURCES}
)

# 创建静态库
add_library(${PROJECT_NAME} STATIC ${MOUDLE_SRC})
