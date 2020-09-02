macro (set_cmake_evironment_variable)
    if (IOS_PLATFORM STREQUAL "OS")
        LINK_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr/lib
            ${PROJECT_SOURCE_DIR}/lib/ios)
        INCLUDE_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr/include
            ${PROJECT_SOURCE_DIR}/src/include)
        set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/sysroot/ios/bin/${IOS_PLATFORM})
        set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/sysroot/ios/lib/${IOS_PLATFORM})
        set (CMAKE_INSTALL_PREFIX /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr)
        SET (ExternalLibs ${ExternalLibs}  -force_load ${LIBRARY_OUTPUT_PATH}/libobject.a)
    elseif (IOS_PLATFORM STREQUAL "SIMULATOR64")
        if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
            set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/SIMULATOR64)
        endif ()
        LINK_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/lib
            ${CMAKE_INSTALL_PREFIX}/lib)
        INCLUDE_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/include
            ${CMAKE_INSTALL_PREFIX}/include
            ${PROJECT_SOURCE_DIR}/src/include)
        set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
        set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
        set (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DIOS_PLATFORM=SIMULATOR64 -DCMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/mk/ios.toolchain.cmake)
        set (SYS_LIBRARY_PATH /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/lib )
        SET (ExternalLibs ${ExternalLibs}  -force_load ${LIBRARY_OUTPUT_PATH}/libobject.a)

    else()
        message(FATAL_ERROR "Invalid IOS_PLATFORM: ${IOS_PLATFORM}")
    endif()

endmacro()

macro (display_ios_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
