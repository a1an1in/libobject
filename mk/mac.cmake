macro (set_cmake_evironment_variable)
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
        set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/mac)
    endif ()
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
    LINK_DIRECTORIES(/usr/local/lib 
        /usr/lib
        ${CMAKE_INSTALL_PREFIX}/lib
        /usr/local/Cellar/mysql/8.0.16/lib
        /usr/local/Cellar/openssl@1.1/1.1.1i/lib
    )

    INCLUDE_DIRECTORIES(/usr/local/include
        /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include
        /usr/local/Cellar/mysql/8.0.16/include/mysql
        /usr/local/Cellar/openssl@1.1/1.1.1i/include
        ${PROJECT_SOURCE_DIR}/src/include
        ${CMAKE_INSTALL_PREFIX}/include
        ${PROJECT_SOURCE_DIR}/sysroot/mac/include)

    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
    set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
    set (EXTERNAL_LIB_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/mac)
    set (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
    SET(ExternalLibs ${ExternalLibs}  -force_load ${LIBRARY_OUTPUT_PATH}/libobject.a crypto)
endmacro()

macro (display_mac_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
