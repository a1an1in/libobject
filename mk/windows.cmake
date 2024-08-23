macro (set_cmake_evironment_variable)
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "C:/Program Files (x86)/libobject")
        SET (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/windows)
    endif ()
    LINK_DIRECTORIES(
        C:/mingw64/lib
        C:/mingw64/x86_64-w64-mingw32/lib
        "C:/Program Files (x86)/Microsoft SDKs/Windows Kits/10/ExtensionSDKs/Microsoft.UniversalCRT.Debug/10.0.19041.0/Redist/Debug/x64"
        # C:/Windows/System32
        ${CMAKE_INSTALL_PREFIX}/lib)

    INCLUDE_DIRECTORIES(
        C:/mingw64/include
        C:/mingw64/x86_64-w64-mingw32/include
        ${PROJECT_SOURCE_DIR}/src/include
        ${CMAKE_INSTALL_PREFIX}/include)

    SET (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
    SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
    SET (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
    SET (ExternalLibs ${ExternalLibs} 
        -Wl,--whole-archive
            object-tests
            object-mockery
            object-upgrader
            object-node 
            # object-compress 
            object-scripts 
            object-stub 
            # object-db 
            object-net
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
        dl 
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

macro (add_module_lists)
    SET (module_lists "")
    list(APPEND module_lists "src/core")
    list(APPEND module_lists "src/scripts")
    list(APPEND module_lists "src/argument")
    list(APPEND module_lists "src/mockery")
    list(APPEND module_lists "src/concurrent")
    list(APPEND module_lists "src/stub")
    list(APPEND module_lists "src/encoding")
    list(APPEND module_lists "src/net")
    list(APPEND module_lists "src/node")
    list(APPEND module_lists "src/upgrader")
    list(APPEND module_lists "tests")
    list(APPEND module_lists "src/.")
    list(APPEND module_lists "3rd/test_lib")
    list(APPEND module_lists "3rd/test_http_plugin")
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
    add_definitions(-Wno-int-to-pointer-cast)
    
endmacro()