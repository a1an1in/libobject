cmake_minimum_required (VERSION 2.8)
project (libobject)

include (mk/util.cmake)
#FIND_LIBRARY(/usr/local/lib /usr/lib NO_DEFAULT_PATH)
LINK_DIRECTORIES(/usr/local/lib /usr/lib ${PROJECT_SOURCE_DIR}/lib)

#message("${Source_Files}")
#FIND_LIBRARY(LIB_SDL_PATH NAMES SDL2)
#message("LIB_SDL_PATH:${LIB_SDL_PATH}")

find_package(PkgConfig)
find_package(opengl)
find_package(CURL)

option (MODULE_UI "ui module" OFF)
option (MODULE_BUS "bus module" OFF)

# The version number.
set (LIBOBJECT_VERSION_MAJOR 1)
set (LIBOBJECT_VERSION_MINOR 0)
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
add_definitions(-O0)
add_definitions(-fPIC)
add_definitions("-Wall -Wno-return-type -Wno-unused-variable -Wno-unused-function
                 -Wno-sometimes-uninitialized -Wno-char-subscripts -Wno-sometimes-uninitialized 
                 -Wno-unused-label -Wno-uninitialized -Wno-int-conversion -Wno-implicit-function-declaration
                 -Wno-uninitialized -g -ggdb")

include_directories(src/include)
#add_subdirectory(src)
find_source_files(Source_Files)
find_main_file(Main_File)

if (Main_File)
    list( REMOVE_ITEM Source_Files ${Main_File} )
    message("-- Main file path: ${Main_File}")
endif ()

if (MODULE_UI)
    message("-- Turn ui module on")
    find_package(SDL2 REQUIRED)
else ()
    message("-- Turn ui module off")
    find_module_files(Exclude_UI_Module_Files ui)
    #message("${Exclude_UI_Module_Files}")
    exclude_files(Source_Files "${Source_Files}" "${Exclude_UI_Module_Files}")
    #message("sources exclude ui module: ${Source_Files}")
endif ()

if (MODULE_BUS)
    message("-- Turn bus module on")
    find_package(SDL2 REQUIRED)
else ()
    message("-- Turn bus module off")
    find_module_files(Exclude_BUS_Module_Files bus)
    #message("${Exclude_UI_Module_Files}")
    exclude_files(Source_Files "${Source_Files}" "${Exclude_BUS_Module_Files}")
    #message("sources exclude bus module: ${Source_Files}")
endif ()


#message("Main_File:${Main_File}")
#message("Source_Files:${Source_Files}")

ADD_LIBRARY (object SHARED ${Source_Files})
target_link_libraries(object SDL2)
target_link_libraries(object SDL2_ttf)

#add_executable(test ${Main_File})
#target_link_libraries(test object)

add_executable(test ${Main_File} ${Source_Files})

#ADD_LIBRARY (object_static STATIC ${Source_Files})
#SET_TARGET_PROPERTIES(object_static PROPERTIES OUTPUT_NAME "object")
#target_link_libraries(object_static SDL2)
#target_link_libraries(object_static SDL2_ttf)

#set_target_properties(object PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(object_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES (object PROPERTIES VERSION 1.2 SOVERSION 1)


install (TARGETS test DESTINATION bin)
install (TARGETS object DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)

