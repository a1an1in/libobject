macro (set_cmake_evironment_variable)
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
        set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/linux)
    endif ()
    LINK_DIRECTORIES(/usr/local/lib 
        /usr/lib
        ${CMAKE_INSTALL_PREFIX}/lib
        ${CMAKE_INSTALL_PREFIX}/lib64)

    INCLUDE_DIRECTORIES(
        /usr/include
        /usr/include/mysql
        /usr/include/python3.8
        /usr/local/include
        ${PROJECT_SOURCE_DIR}/src/include
        ${CMAKE_INSTALL_PREFIX}/include)

    SET (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
    SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
    SET (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
    SET (ExternalLibs ${ExternalLibs} 
        -Wl,--whole-archive
            object-tests
            object-mockery
            object-drivers
            object-node 
            object-archive 
            object-compress 
            object-scripts 
            object-attacher 
            object-stub 
            object-db 
            object-net
            object-concurrent
            object-crypto
            object-encoding
            object-argument
            object-core
        -Wl,--no-whole-archive
        mysqlclient dl pthread m z)
endmacro()

macro (display_linux_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()

macro (add_module_lists)
    SET (module_lists "")
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
    list(APPEND module_lists "3rd/attacher-builtin")
    list(APPEND module_lists "3rd/test_process")
    list(APPEND module_lists "3rd/testlib")
    list(APPEND module_lists "3rd/test_http_plugin")
    
    #list(APPEND module_lists "src/media")
    #list(APPEND module_lists "src/ui")
    #list(APPEND module_lists "3rd/app")

    list(APPEND module_lists "tests")

    list(APPEND module_lists "src/.")
    add_subdirectories("${module_lists}")
endmacro()

# add compile paramater 
macro (add_compiling_options)
    #add_definitions(-DDEBUG)
    #add_definitions(-O0 -g)
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