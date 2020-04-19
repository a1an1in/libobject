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
        ${LIBRARY_OUTPUT_PATH})

    INCLUDE_DIRECTORIES(
        ${CMAKE_ANDROID_NDK}/sysroot/usr/include
        ${CMAKE_INSTALL_PREFIX}/include
        ${PROJECT_SOURCE_DIR}/src/include)

endmacro()

macro (display_android_platform_configs)
    message("-- android platform configs:")
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
    message("-- CMAKE_ANDROID_NDK: ${CMAKE_ANDROID_NDK}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
