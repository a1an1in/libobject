cmake_minimum_required (VERSION 2.8)
include (mk/util.cmake)
init_platform()
project (libobject)
config_platform()

#if (APPLE)
    #message("app platform")
#endif ()

# The version number.
# Version format major.minor.maintenance.build
set (LIBOBJECT_VERSION_MAJOR 2)
set (LIBOBJECT_VERSION_MINOR 1)
set (LIBOBJECT_VERSION_MAINTENANCE 0)
set (LIBOBJECT_VERSION_BUILD 1)

configure_file (
    "${PROJECT_SOURCE_DIR}/src/include/libobject/version.h.in"
    "${PROJECT_SOURCE_DIR}/src/include/libobject/version.h")

# Set the output folder where your program will be created
set (CMAKE_MACOSX_RPATH 1)
# Debug/Release settings
set (CMAKE_BUILD_TYPE "Debug")

display_configs("${PLATFORM}")

#find_package(PkgConfig)
#find_package(opengl)
#find_package(CURL)

option (MODULE_UI "ui module" OFF)
option (MODULE_BUS "bus module" OFF)

# add compile paramater 
add_definitions(-DDEBUG)
add_definitions(-O0 -g)
add_definitions(-fPIC)
#add_definitions(-Wl,--no-whole-archive)
#add_definitions(-Wl,--whole-archive)
add_definitions("-Wall -Wno-return-type -Wno-unused-variable -Wno-unused-function
                 -Wno-sometimes-uninitialized -Wno-char-subscripts -Wno-sometimes-uninitialized 
                 -Wno-unused-label -Wno-uninitialized -Wno-int-conversion -Wno-implicit-function-declaration
                 -Wno-uninitialized -Wno-nullability-completeness -Wno-expansion-to-defined")

#add_subdirectory(src)
find_source_files(Source_Files)
find_main_file(Main_File)

if (Main_File)
    list(REMOVE_ITEM Source_Files ${Main_File} )
    message("-- Main file path: ${Main_File}")
endif ()

if (MODULE_UI)
    message("-- Turn ui module on")
    #find_package(SDL2 REQUIRED)
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

#add_library (object static ${source_files})
#target_link_libraries(object ${ExternalLibs})
#message("-- ExternalLibs :${ExternalLibs}")

#add_executable(test ${Main_File})
#target_link_libraries(test -force_load object)

ADD_LIBRARY (object SHARED ${Source_Files})
target_link_libraries(object ${ExternalLibs})

ADD_LIBRARY (object_static STATIC ${Source_Files})
target_link_libraries(object_static ${ExternalLibs})

add_executable(test ${Main_File})
target_link_libraries(test object)
ADD_DEPENDENCIES(test object)

#set_target_properties(object PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(object_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES (object PROPERTIES VERSION 1.2 SOVERSION 1)
SET_TARGET_PROPERTIES (object_static PROPERTIES OUTPUT_NAME "object")

install (TARGETS test DESTINATION bin)
install (TARGETS object DESTINATION lib)
install (TARGETS object_static DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)

