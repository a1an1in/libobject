macro (set_cmake_evironment_variable)
    LINK_DIRECTORIES(/usr/local/lib 
        /usr/lib
        ${PROJECT_SOURCE_DIR}/lib)

    INCLUDE_DIRECTORIES(/usr/local/include
        /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include
        ${PROJECT_SOURCE_DIR}/src/include)

    set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/mac)
    set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/mac)
    set (LIBRARY_DIR_PATH ${PROJECT_SOURCE_DIR}/lib/mac/ )
endmacro()

macro (display_mac_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
