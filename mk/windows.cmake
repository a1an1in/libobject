macro (set_cmake_evironment_variable)
    set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/windows)
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
    LINK_DIRECTORIES(
            $ENV{MinGW_PATH}/lib
            ${CMAKE_INSTALL_PREFIX}/lib)

    INCLUDE_DIRECTORIES(
            $ENV{MinGW_PATH}/include
            ${PROJECT_SOURCE_DIR}/src/include
            ${CMAKE_INSTALL_PREFIX}/include
            ${PROJECT_SOURCE_DIR}/sysroot/windows/include)

    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
    set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
    set (EXTERNAL_LIB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/windows)
    set (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
    SET(ExternalLibs ${ExternalLibs} -Wl,--whole-archive object -Wl,--no-whole-archive regex ws2_32 pthread m)
endmacro()

macro (display_windows_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
