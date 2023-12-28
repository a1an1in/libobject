cmake_minimum_required (VERSION 3.0)
include (mk/util.cmake)
init_platform()

if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)
project (libobject VERSION 2.11.2.184)

#version format "[VERSION <major>[.<minor>[.<patch>[.<tweak>]]]]"
configure_file(
    ${PROJECT_SOURCE_DIR}/src/include/libobject/version.h.in
    ${PROJECT_SOURCE_DIR}/src/include/libobject/version.h
    @ONLY
)

config_platform()

set (CMAKE_MACOSX_RPATH 1)
set (CMAKE_BUILD_TYPE "Debug")

display_configs("${PLATFORM}")
add_compiling_options()
#include (mk/build_external.cmake)

set (module_lists "")
list(APPEND module_lists "src/argument")
list(APPEND module_lists "src/cmds")
list(APPEND module_lists "src/concurrent")
list(APPEND module_lists "src/core")
list(APPEND module_lists "src/stub")
list(APPEND module_lists "src/encoding")

if ("${PLATFORM}" STREQUAL "windows")
    list(APPEND module_lists "tests")
    #list(APPEND module_lists "src/net")
else()
    list(APPEND module_lists "src/compress")
    list(APPEND module_lists "src/crypto")
    list(APPEND module_lists "src/ctest")
    list(APPEND module_lists "src/database")
    
    list(APPEND module_lists "src/attacher")
    list(APPEND module_lists "src/message")
    list(APPEND module_lists "src/net")
    list(APPEND module_lists "src/scripts")
    list(APPEND module_lists "src/archive")
    list(APPEND module_lists "3rd/test_process")
    list(APPEND module_lists "3rd/test_lib")
    list(APPEND module_lists "3rd/test_lib2")
    
    #list(APPEND module_lists "src/media")
    #list(APPEND module_lists "src/ui")
    #list(APPEND module_lists "3rd/app")

    list(APPEND module_lists "tests")
endif()

list(APPEND module_lists "src/.")
add_subdirectories("${module_lists}")

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)

