macro (set_cmake_evironment_variable)
    if (IOS_PLATFORM STREQUAL "OS")
        LINK_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr/lib
            ${PROJECT_SOURCE_DIR}/lib/ios)
        INCLUDE_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr/include
            ${PROJECT_SOURCE_DIR}/src/include)
        set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/ios/${IOS_PLATFORM})
        set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/ios/${IOS_PLATFORM})
        set (CMAKE_INSTALL_PREFIX /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr)
    elseif (IOS_PLATFORM STREQUAL "SIMULATOR64")
        LINK_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/lib
            ${PROJECT_SOURCE_DIR}/lib/ios)
        INCLUDE_DIRECTORIES(
            /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr/include
            ${PROJECT_SOURCE_DIR}/src/include)
        set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/ios/${IOS_PLATFORM})
        set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/ios/${IOS_PLATFORM})
        set (CMAKE_INSTALL_PREFIX /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk/usr)
    else()
        message(FATAL_ERROR "Invalid IOS_PLATFORM: ${IOS_PLATFORM}")
    endif()

endmacro()

macro (display_ios_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
