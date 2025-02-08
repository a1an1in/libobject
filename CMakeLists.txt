cmake_minimum_required (VERSION 3.0)
include (mk/util.cmake)
init_platform()

if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project (libobject VERSION 2.13.2.910)

config_lib_version()
config_platform()

set (CMAKE_MACOSX_RPATH 1)
set (CMAKE_BUILD_TYPE "Debug")

display_configs("${PLATFORM}")
add_compiling_options()
include (mk/build_external.cmake)
add_module_lists()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)