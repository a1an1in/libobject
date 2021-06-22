#set (PLATFORM windows)
cmake_minimum_required (VERSION 2.8)
include (mk/util.cmake)
init_platform()
project (cean)
config_platform()

set (CMAKE_MACOSX_RPATH 1)
set (CMAKE_BUILD_TYPE "Debug")
display_configs("${PLATFORM}")

# add compile paramater 
config_build_options()

add_subdirectory(src)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)

