macro (set_android_environment_variable)
    set(CMAKE_SYSTEM_NAME Android)
    set(CMAKE_SYSTEM_VERSION 22) # API level
    #set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a)
    #set(ANDROID_ABI arm64-v8a)
    #set(ANDROID_ABI x86_64)
    #set(ANDROID_ABI x86)
    #set(ANDROID_ABI armeabi-v7a)
    message("********android_abi: ${ANDROID_ABI}")
    if ("${ANDROID_ABI}" STREQUAL "")
        set(ANDROID_ABI armeabi-v7a)
    endif()
    set(CMAKE_ANDROID_STL_TYPE gnustl_static)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_ANDROID_NDK}/build/cmake/android.toolchain.cmake)

    SET(ExternalLibs ${ExternalLibs} log)
endmacro()

macro (set_cmake_evironment_variable)
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
        set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/android)
    endif ()
    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin/${ANDROID_ABI})
    set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib/${ANDROID_ABI})
    if ("${ANDROID_ABI}" STREQUAL "x86")
        set (ARCH_NAME arch-x86)
    elseif ("${ANDROID_ABI}" STREQUAL "x86_64")
        set (ARCH_NAME arch-x86_64)
    elseif ("${ANDROID_ABI}" STREQUAL "arm64-v8a")
        set (ARCH_NAME arch-arm64)
    elseif ("${ANDROID_ABI}" STREQUAL "armeabi-v7a")
        set (ARCH_NAME arch-arm)
    endif ()

    LINK_DIRECTORIES(
        ${CMAKE_ANDROID_NDK}/platforms/android-21/${ARCH_NAME}/usr/lib
        ${CMAKE_ANDROID_NDK}/sysroot/usr/lib
        ${LIBRARY_OUTPUT_PATH})

    INCLUDE_DIRECTORIES(
        ${CMAKE_ANDROID_NDK}/sysroot/usr/include
        ${CMAKE_INSTALL_PREFIX}/include
        ${PROJECT_SOURCE_DIR}/src/include)

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
    list(APPEND module_lists "src/database")
    list(APPEND module_lists "src/attacher")
    list(APPEND module_lists "src/scripts")
    list(APPEND module_lists "src/archive")

    list(APPEND module_lists "src/drivers")
    list(APPEND module_lists "3rd/test_process")
    list(APPEND module_lists "3rd/test_lib")
    list(APPEND module_lists "3rd/test_lib2")
    
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