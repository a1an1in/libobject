cmake_minimum_required (VERSION 2.8)
include (mk/util.cmake)
init_platform()
project (cean)
config_platform()

set (CMAKE_MACOSX_RPATH 1)
set (CMAKE_BUILD_TYPE "Debug")

display_configs("${PLATFORM}")

# add compile paramater 
add_definitions(-DDEBUG)
add_definitions(-O0 -g)
add_definitions(-fPIC)
add_definitions(-Wall)
add_definitions(-Wno-return-type)
add_definitions(-Wno-unused-variable)
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

add_subdirectory(src)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)

