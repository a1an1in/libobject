macro (set_cmake_evironment_variable)
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
        set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/linux)
    endif ()
    LINK_DIRECTORIES(/usr/local/lib 
        /usr/lib
        ${CMAKE_INSTALL_PREFIX}/lib)

    INCLUDE_DIRECTORIES(
        /usr/include
        /usr/include/mysql
        /usr/local/include
        ${PROJECT_SOURCE_DIR}/src/include
        ${CMAKE_INSTALL_PREFIX}/include)

    set (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)
    set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
    SET (ExternalLibs ${ExternalLibs} -Wl,--whole-archive object-ex-tests object-ex-scripts object-ex-stub object-ex-db object-ex-net object -Wl,--no-whole-archive crypto dl pthread m)
endmacro()

macro (display_linux_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
