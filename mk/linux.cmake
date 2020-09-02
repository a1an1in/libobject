macro (set_cmake_evironment_variable)
    LINK_DIRECTORIES(/usr/local/lib 
        /usr/lib
        ${PROJECT_SOURCE_DIR}/lib)

    INCLUDE_DIRECTORIES(/usr/local/include
        ${PROJECT_SOURCE_DIR}/src/include)

    SET(ExternalLibs ${ExternalLibs} pthread m -Wl,--whole-archive object -Wl,--no-whole-archive)

    set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/sysroot/linux/bin)
    set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/sysroot/linux/lib)
endmacro()

macro (display_linux_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
