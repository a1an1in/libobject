macro (set_cmake_evironment_variable)
    if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
        set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/sysroot/linux)
    endif ()

    LINK_DIRECTORIES(/usr/local/lib 
        /usr/lib
        ${CMAKE_INSTALL_PREFIX}/lib)

    INCLUDE_DIRECTORIES(/usr/local/include
        ${PROJECT_SOURCE_DIR}/src/include)
    
    set (BUILD_EXTERNAL_ARGS -DPLATFORM=${PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
    SET(ExternalLibs ${ExternalLibs} -Wl,--whole-archive object -Wl,--no-whole-archive pthread m )

    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/bin)  
    set (LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/lib)
endmacro()

macro (display_linux_platform_configs)
    message("-- EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
    message("-- LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
    message("-- CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
endmacro()
