cmake_minimum_required (VERSION 2.8)
include (mk/util.cmake)
init_platform("${PLATFORM}")
project (libobject)
config_platform("${PLATFORM}")

# The version number.
set (LIBOBJECT_VERSION_MAJOR 1)
set (LIBOBJECT_VERSION_MINOR 0)
# Set the output folder where your program will be created
set (CMAKE_MACOSX_RPATH 1)
# Debug/Release settings
set (CMAKE_BUILD_TYPE "Debug")

message("EXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
message("LIBRARY_OUTPUT_PATH: ${LIBRARY_OUTPUT_PATH}")
message("CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

find_package(PkgConfig)
find_package(opengl)
find_package(CURL)

option (MODULE_UI "ui module" OFF)
option (MODULE_BUS "bus module" OFF)

# add compile paramater 
add_definitions(-DDEBUG)
add_definitions(-O0 -g)
add_definitions(-fPIC)
add_definitions("-Wall -Wno-return-type -Wno-unused-variable -Wno-unused-function
                 -Wno-sometimes-uninitialized -Wno-char-subscripts -Wno-sometimes-uninitialized 
                 -Wno-unused-label -Wno-uninitialized -Wno-int-conversion -Wno-implicit-function-declaration
                 -Wno-uninitialized")

#add_subdirectory(src)
find_source_files(Source_Files)
find_main_file(Main_File)

if (Main_File)
    list(REMOVE_ITEM Source_Files ${Main_File} )
    message("-- Main file path: ${Main_File}")
endif ()

if (MODULE_UI)
    message("-- Turn ui module on")
    find_package(SDL2 REQUIRED)
    SET(ExternalLibs ${ExternalLibs} SDL2 SDL2_ttf)
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

ADD_LIBRARY (object SHARED ${Source_Files})
target_link_libraries(object ${ExternalLibs})

add_executable(test ${Main_File})
target_link_libraries(test object)

#add_executable(test ${Main_File} ${Source_Files})
#target_link_libraries(test ${ExternalLibs})

#set_target_properties(object PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(object_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES (object PROPERTIES VERSION 1.2 SOVERSION 1)


install (TARGETS test DESTINATION bin)
install (TARGETS object DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)

