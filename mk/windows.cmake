macro (set_cmake_evironment_variable)
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "C:/Program Files (x86)/libobject")
        set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/windows)
    endif ()
    LINK_DIRECTORIES(
        C:/mingw64/lib
        C:/mingw64/x86_64-w64-mingw32/lib
        "C:/Program Files (x86)/Microsoft SDKs/Windows Kits/10/ExtensionSDKs/Microsoft.UniversalCRT.Debug/10.0.19041.0/Redist/Debug/x64"
        # C:/Windows/System32
        ${CMAKE_INSTALL_PREFIX}/lib)

    INCLUDE_DIRECTORIES(
        C:/mingw64/include
        ${PROJECT_SOURCE_DIR}/src/include
        ${CMAKE_INSTALL_PREFIX}/include)

    set (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
    set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
    SET (ExternalLibs ${ExternalLibs} 
        -Wl,--whole-archive
            object-tests
            object-mockery
            # object-compress 
            # object-scripts 
            object-stub 
            # object-db 
            # object-net
            object-concurrent
            # object-crypto
            object-encoding
            object-argument
            object-core
        -Wl,--no-whole-archive
        ws2_32
        wsock32
        regex
        # api-ms-win-crt-stdio-l1-1-0
        # ucrtbased
        # cmt
        # ucrt
        # mysqlclient
        # python3.8
        # crypto 
        # dl 
        pthread 
        # m 
        # msvcrt
        # z
        )
endmacro()

macro (display_windows_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
