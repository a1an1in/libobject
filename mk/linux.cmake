macro (set_cmake_evironment_variable)
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
        set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/linux)
    endif ()

    # 根据平台动态设置库变量
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        message(STATUS "x86_64 platform detected.")
        set(COMPRESS_LIB "object-compress")  # x86_64 平台链接 object-compress
        set(ATTACHER_LIB "object-attacher")  # x86_64 平台链接 object-attacher
        set(DATABASE_LIB "object-db")        # x86_64 平台链接 object-db
        set(MYSQL_LIB "mysqlclient")         # x86_64 平台链接 mysqlclient
        set(Z_LIB "z")                       # x86_64 平台链接 z
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        message(STATUS "ARM platform detected.")
        set(COMPRESS_LIB "")  # ARM 平台不链接 object-compress
        set(ATTACHER_LIB "")  # ARM 平台不链接 object-attacher
        set(DATABASE_LIB "")  # ARM 平台不链接 object-db
        set(MYSQL_LIB "")     # ARM 平台不链接 mysqlclient
        set(Z_LIB "")         # ARM 平台不链接 z
    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    SET (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})

    # 动态设置 ExternalLibs
    SET (ExternalLibs
        -Wl,--whole-archive
            object-tests
            object-mockery
            object-drivers
            object-node
            object-archive
            ${COMPRESS_LIB} # 动态控制是否链接 object-compress
            object-scripts
            ${ATTACHER_LIB} # 动态控制是否链接 object-attacher
            object-stub
            ${DATABASE_LIB} # 动态控制是否链接 object-db
            object-net
            object-concurrent
            object-crypto
            object-encoding
            object-argument
            object-core
        -Wl,--no-whole-archive
        ${MYSQL_LIB} dl pthread m ${Z_LIB} # 动态控制是否链接 mysqlclient 和 z
    )
endmacro()

macro (display_linux_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()

macro (add_module_lists)
    SET (module_lists
        "src/argument"
        "src/mockery"
        "src/concurrent"
        "src/core"
        "src/stub"
        "src/encoding"
        "src/net"
        "src/node"
        "src/crypto"
        "src/scripts"
        "src/archive"
        "src/drivers"
        "3rd/attacher-builtin"
        "3rd/test_process"
        "3rd/testlib"
        "3rd/test_http_plugin"
        "tests"
        "src/."
    )

    # 检测是否为 ARM 平台
    if (NOT CMAKE_SYSTEM_PROCESSOR MATCHES "arm" AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        list(APPEND module_lists "src/attacher") # 非 ARM 平台添加 attacher
        list(APPEND module_lists "src/database") # 非 ARM 平台添加 database
        list(APPEND module_lists "src/compress") # 非 ARM 平台添加 compress
    else()
        message(STATUS "ARM platform detected. Skipping attacher, database, and compress modules.")
    endif()

    add_subdirectories("${module_lists}")
endmacro()

macro (add_compiling_options)
    add_definitions(
        -fPIC
        -Wall
        -Wno-return-type
        -Wno-unused-variable
        -Wno-unused-but-set-variable
        -Wno-unused-function
        -Wno-unused-label
        -Wno-uninitialized
        -Wno-int-conversion
        -Wno-implicit-function-declaration
        -Wno-nullability-completeness
        -Wno-expansion-to-defined
        -Wno-deprecated-declarations
        -Wno-pointer-sign
        -Wno-incompatible-pointer-types
        -Wno-pointer-to-int-cast
        -Wno-address-of-packed-member
    )
endmacro()