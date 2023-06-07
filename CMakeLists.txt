cmake_minimum_required (VERSION 3.0)
include (mk/util.cmake)
init_platform()

if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)
project (libobject VERSION 2.7.4.1)

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

# add compile paramater 
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
add_definitions(-Wno-uninitialized)
add_definitions(-Wno-nullability-completeness)
add_definitions(-Wno-expansion-to-defined)
add_definitions(-Wno-deprecated-declarations)
add_definitions(-Wno-pointer-sign)
add_definitions(-Wno-incompatible-pointer-types)
add_definitions(-Wno-pointer-to-int-cast)

#include (mk/build_external.cmake)

set (module_lists "")

if ("${PLATFORM}" STREQUAL "windows")
    list(APPEND module_lists "src/argument")
    list(APPEND module_lists "src/cmds")
    list(APPEND module_lists "src/concurrent")
    list(APPEND module_lists "src/core")
    # list(APPEND module_lists "src/net")
    list(APPEND module_lists "src/stub")
    list(APPEND module_lists "tests")
else()
    #list(APPEND module_lists "src/archive")
    list(APPEND module_lists "src/argument")
    list(APPEND module_lists "src/cmds")
    list(APPEND module_lists "src/compress")
    list(APPEND module_lists "src/concurrent")
    list(APPEND module_lists "src/core")
    list(APPEND module_lists "src/crypto")
    list(APPEND module_lists "src/ctest")
    list(APPEND module_lists "src/database")
    list(APPEND module_lists "src/encoding")
    #list(APPEND module_lists "src/media")
    list(APPEND module_lists "src/message")
    list(APPEND module_lists "src/net")
    list(APPEND module_lists "src/scripts")
    list(APPEND module_lists "src/stub")
    #list(APPEND module_lists "src/ui")

    # list(APPEND module_lists "3rd/test_lib")
    # list(APPEND module_lists "3rd/test_attach")
    #list(APPEND module_lists "3rd/app")
    list(APPEND module_lists "tests")
endif()

list(APPEND module_lists "src/.")

add_subdirectories("${module_lists}")

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)

