# Set the Android ABI (architecture)
if(NOT DEFINED ANDROID_ABI)
    set(ANDROID_ABI "arm64-v8a" CACHE STRING "Android ABI (e.g., arm64-v8a, armeabi-v7a, x86, x86_64)")
endif()

# Set the Android platform version
if(NOT DEFINED ANDROID_PLATFORM)
    set(ANDROID_PLATFORM "android-21" CACHE STRING "Android platform version (e.g., android-21)")
endif()

# Additional configurations
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX})

message(STATUS "Android ABI: ${ANDROID_ABI}")
message(STATUS "Android platform: ${ANDROID_PLATFORM}")

macro (set_cmake_evironment_variable)
    LINK_DIRECTORIES(
        ${CMAKE_ANDROID_NDK}/platforms/android-21/${ARCH_NAME}/usr/lib
        ${CMAKE_ANDROID_NDK}/sysroot/usr/lib
        ${LIBRARY_OUTPUT_PATH})

    INCLUDE_DIRECTORIES(
        ${CMAKE_ANDROID_NDK}/sysroot/usr/include
        ${CMAKE_INSTALL_PREFIX}/include
        ${PROJECT_SOURCE_DIR}/src/include)

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
        ${MYSQL_LIB} dl m ${Z_LIB} # 动态控制是否链接 mysqlclient 和 z
    )

    set (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DANDROID_ABI=armeabi-v7a -DCMAKE_ANDROID_NDK=${CMAKE_ANDROID_NDK})

endmacro()

macro (display_android_platform_configs)
    message("-- android platform configs:")
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
    message("-- CMAKE_ANDROID_NDK: ${CMAKE_ANDROID_NDK}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()

macro (add_module_lists)
    set (module_lists "")
    list(APPEND module_lists "src/argument")
    list(APPEND module_lists "src/mockery")
    list(APPEND module_lists "src/concurrent")
    list(APPEND module_lists "src/core")
    list(APPEND module_lists "src/stub")
    list(APPEND module_lists "src/encoding")
    list(APPEND module_lists "src/net")
    list(APPEND module_lists "src/node")

    list(APPEND module_lists "src/compress")
    list(APPEND module_lists "src/crypto")
    #list(APPEND module_lists "src/attacher")
    list(APPEND module_lists "src/scripts")
    list(APPEND module_lists "src/archive")

    list(APPEND module_lists "src/drivers")
    list(APPEND module_lists "3rd/test_process")
    list(APPEND module_lists "3rd/attacher-builtin")
    list(APPEND module_lists "3rd/testlib")
    
    #list(APPEND module_lists "src/media")
    #list(APPEND module_lists "src/ui")
    #list(APPEND module_lists "3rd/app")

    list(APPEND module_lists "tests")

    list(APPEND module_lists "src/.")
    add_subdirectories("${module_lists}")
endmacro()

# add compile paramater 
macro (add_compiling_options)
    add_definitions(-DDEBUG)
    add_definitions(-O0 -g)
    add_definitions(-fPIC)
    #add_definitions(-Wl,--no-whole-archive)
    #add_definitions(-Wl,--whole-archive)
    add_definitions(-Wall)
    add_definitions(-Wno-return-type)
    add_definitions(-Wno-unused-variable)
    add_definitions(-Wno-unused-but-set-variable)
    add_definitions(-Wno-unused-function)
    add_definitions(-Wno-sometimes-uninitialized)
    add_definitions(-Wno-char-subscripts)
    add_definitions(-Wno-sometimes-uninitialized)
    add_definitions(-Wno-unused-label)
    add_definitions(-Wno-uninitialized)
    add_definitions(-Wno-int-conversion)
    add_definitions(-Wno-implicit-function-declaration)
    add_definitions(-Wno-nullability-completeness)
    add_definitions(-Wno-expansion-to-defined)
    add_definitions(-Wno-deprecated-declarations)
    add_definitions(-Wno-pointer-sign)
    add_definitions(-Wno-incompatible-pointer-types)
    add_definitions(-Wno-pointer-to-int-cast)
    add_definitions(-Wno-address-of-packed-member)
endmacro()