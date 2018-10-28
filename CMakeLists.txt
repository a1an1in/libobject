cmake_minimum_required (VERSION 2.8)
project (libobject)

#FIND_LIBRARY(/usr/local/lib /usr/lib NO_DEFAULT_PATH)
LINK_DIRECTORIES(/usr/local/lib /usr/lib ${PROJECT_SOURCE_DIR}/lib)

#FIND_LIBRARY(LIB_SDL_PATH NAMES SDL2)
#message("LIB_SDL_PATH:${LIB_SDL_PATH}")

find_package(PkgConfig)
find_package(SDL2 REQUIRED)
find_package(opengl)
find_package(CURL)

#option (USE_MYMATH
        #"Use tutorial provided math implementation" ON)
        

# The version number.
set (LIBOBJECT_VERSION_MAJOR 1)
set (LIBOBJECT_VERSION_MINOR 0)

set (Source_Files)
# Set the output folder where your program will be created
set (CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/bin)
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set (CMAKE_MACOSX_RPATH 1)
# Debug/Release settings
set (CMAKE_BUILD_TYPE "Debug")

# add compile paramater 
add_definitions(-DDEBUG)
add_definitions(-O0 -g -ggdb)
add_definitions(-fPIC)
add_definitions("-Wall -Wno-return-type -Wno-unused-variable -Wno-unused-function
                 -Wno-sometimes-uninitialized -Wno-char-subscripts -Wno-sometimes-uninitialized 
                 -Wno-unused-label -Wno-uninitialized -Wno-int-conversion -Wno-implicit-function-declaration
                 -Wno-uninitialized")
include_directories(src/include)
add_subdirectory(src)

file (GLOB_RECURSE Main_File main.c)
if (Main_File)
  list( REMOVE_ITEM Source_Files ${Main_File} )
endif()

#message("Main_File:${Main_File}")
#message("Source_Files:${Source_Files}")

ADD_LIBRARY (object SHARED ${Source_Files})
target_link_libraries(object SDL2)
target_link_libraries(object SDL2_ttf)

add_executable(test ${Main_File})
target_link_libraries(test object)

#ADD_LIBRARY (object_static STATIC ${Source_Files})
#SET_TARGET_PROPERTIES(object_static PROPERTIES OUTPUT_NAME "object")
#target_link_libraries(object_static SDL2)
#target_link_libraries(object_static SDL2_ttf)

# 使动态库和静态库同时存在
#set_target_properties(object PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(object_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# 指定动态库版本
# VERSION 动态库版本
# SOVERSION API版本
SET_TARGET_PROPERTIES (object PROPERTIES VERSION 1.2 SOVERSION 1)


install (TARGETS test DESTINATION bin)
#install (TARGETS object DESTINATION lib)
install (TARGETS object DESTINATION lib)

