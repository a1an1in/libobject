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
set (LIBOBJECT_VERSION_MINOR 4)
set (LIBOBJECT_VERSION_MAINTENANCE 5)
set (LIBOBJECT_VERSION_BUILD 9)

set (VERSION_NAME "${LIBOBJECT_VERSION_MAJOR}.${LIBOBJECT_VERSION_MINOR}.${LIBOBJECT_VERSION_MAINTENANCE}.${LIBOBJECT_VERSION_BUILD}")

message("-- CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("-- CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")
message("-- CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")

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

# add compile paramater 
add_definitions(-DDEBUG)
add_definitions(-O0 -g)
add_definitions(-fPIC)
#add_definitions(-Wl,--no-whole-archive)
#add_definitions(-Wl,--whole-archive)
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
add_definitions(-Wno-nonportable-include-path)

#add_subdirectory(src)
find_source_files(Source_Files)
find_main_file(Main_File)

if (Main_File)
    list(REMOVE_ITEM Source_Files ${Main_File} )
    message("-- Main file path: ${Main_File}")
endif ()


if ("${PLATFORM}" STREQUAL "android")
    ADD_LIBRARY (object SHARED ${Source_Files})
    target_link_libraries(object ${ExternalLibs})

    add_executable(cean ${Main_File})
    target_link_libraries(cean object)
    ADD_DEPENDENCIES(cean object)

    SET_TARGET_PROPERTIES (object PROPERTIES VERSION ${VERSION_NAME} SOVERSION ${LIBOBJECT_VERSION_MAJOR})

    if ("${ANDROID_ABI}" STREQUAL "x86")
        install (TARGETS object DESTINATION ${CMAKE_ANDROID_NDK}/platforms/android-21/${ARCH_NAME}/usr/lib)
        install (TARGETS object DESTINATION ~/workspace/LIBOBJECT/libobject-wrapper/projects/android/demo/app/src/main/jni/lib/${ANDROID_ABI})
    elseif ("${ANDROID_ABI}" STREQUAL "x86_64")
        #install (TARGETS object DESTINATION lib/x86_64-linux-android)
        install (TARGETS object DESTINATION ${CMAKE_ANDROID_NDK}/platforms/android-21/${ARCH_NAME}/usr/lib64)
        install (TARGETS object DESTINATION ~/workspace/LIBOBJECT/libobject-wrapper/projects/android/demo/app/src/main/jni/lib/${ANDROID_ABI})
    elseif ("${ANDROID_ABI}" STREQUAL "arm64-v8a")
        #install (TARGETS object DESTINATION lib/aarch64-linux-android)
        install (TARGETS object DESTINATION ${CMAKE_ANDROID_NDK}/platforms/android-21/${ARCH_NAME}/usr/lib64)
    elseif ("${ANDROID_ABI}" STREQUAL "armeabi-v7a")
        #install (TARGETS object DESTINATION lib/arm-linux-androideabi)
        install (TARGETS object DESTINATION ${CMAKE_ANDROID_NDK}/platforms/android-21/${ARCH_NAME}/usr/lib)
        install (TARGETS object DESTINATION ~/workspace/LIBOBJECT/libobject-wrapper/projects/android/demo/app/src/main/jni/lib/${ANDROID_ABI})
        install (TARGETS object DESTINATION ~/workspace/goya-github/android/goya-alone/app/src/main/jni/lib/armeabi-v7a)
    endif ()

    install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)

elseif ("${PLATFORM}" STREQUAL "ios")
    ADD_LIBRARY (object_static STATIC ${Source_Files})
    target_link_libraries(object_static ${ExternalLibs})

    SET_TARGET_PROPERTIES (object_static PROPERTIES VERSION ${VERSION_NAME} SOVERSION ${LIBOBJECT_VERSION_MAJOR})
    SET_TARGET_PROPERTIES (object_static PROPERTIES OUTPUT_NAME "object")

    install (TARGETS object_static DESTINATION lib)
    install (TARGETS object_static DESTINATION /Users/alanlin/workspace/LIBOBJECT/libobject-wrapper/projects/ios/demo/demo/libs/SIMULATOR64)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)
else ()
    ADD_LIBRARY (object SHARED ${Source_Files})
    target_link_libraries(object ${ExternalLibs})

    #ADD_LIBRARY (object_static STATIC ${Source_Files})
    #target_link_libraries(object_static ${ExternalLibs})

    add_executable(cean ${Main_File})
    target_link_libraries(cean object)
    ADD_DEPENDENCIES(cean object)

    #set_target_properties(object PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    #set_target_properties(object_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    SET_TARGET_PROPERTIES (object PROPERTIES VERSION ${VERSION_NAME} SOVERSION ${LIBOBJECT_VERSION_MAJOR})
    #SET_TARGET_PROPERTIES (object_static PROPERTIES OUTPUT_NAME "object")
    install (TARGETS cean DESTINATION bin)
    install (TARGETS object DESTINATION lib)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/libobject DESTINATION include)
endif()

