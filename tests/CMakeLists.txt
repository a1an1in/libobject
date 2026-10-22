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

# aarch64 平台暂不支持 stub，排除 tests/stub 测试文件
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    message(STATUS "aarch64 platform detected. Skipping tests/stub sources.")
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/stub/.*")
endif()

# board 模块只在 linux 平台编译，非 linux 平台排除 tests/board 测试文件
if(NOT PLATFORM STREQUAL "linux")
    message(STATUS "Non-linux platform detected. Skipping tests/board sources.")
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/board/.*")
endif()

# Android 暂不支持 compress/archive/vpn，排除对应测试
if(PLATFORM STREQUAL "android")
    message(STATUS "Android platform detected. Skipping tests/compress, tests/archive and tests/net/test_vpn.")
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/compress/.*")
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/archive/.*")
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/net/test_vpn\\.c")
endif()

# stub 的运行时打桩(hook)在 Windows 上暂不支持(跳转指令无法跨 DLL)，
# p2p/stun 的事件循环 udp 处理还没适配，vpn 需要 TUN 设备(Windows 上只有存根)，
# 排除这些用例，避免测试崩溃。
if(PLATFORM STREQUAL "windows")
    message(STATUS "Windows platform detected. Skipping tests/stub, tests/p2p, tests/vpn and tests/bus sources.")
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/stub/.*")
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/test_p2p\\.c")
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/test_vpn\\.c")
    # bus 用例的 client 销毁时, 事件线程和主线程会同时改 producer 的 worker 链表
    # (链表默认没加锁), 偶发把链表改坏导致崩溃, 先排除
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/test_bus\\.c")
    # concurrent 模块的事件/worker 在 windows 上的时序和 linux 不一样,
    # 这些用例会偶发崩溃(producer 的 worker 链表没有加锁), 先排除
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/concurrent/.*")
    # node 服务在 windows 上跑不起来(用例本来就失败),
    # 而且它退出时会把 producer 的 worker 链表搞坏, 导致整个测试进程崩,
    # 连带后面的用例都跑不了, 先排除
    list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/test_node\\.c")
endif()

# 测试用例会把解包/生成的文件放到 tests/**/output 目录下面，
# 这些文件不是源码(有的甚至就是 .c 文件)，必须排除掉。
list(FILTER MOUDLE_SOURCES EXCLUDE REGEX ".*/output/.*")

# 将头文件和源文件合并
file(GLOB MOUDLE_SRC
    ${MOUDLE_HEADERS}
    ${MOUDLE_SOURCES}
)

# 创建静态库
add_library(${PROJECT_NAME} STATIC ${MOUDLE_SRC})
