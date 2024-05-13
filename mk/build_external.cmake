include(ExternalProject)

macro (build_libobject)
    if(EXISTS ${PROJECT_SOURCE_DIR}/3rd/libobject)
        message("rebuild libobject")
        ExternalProject_Add(libobject
            SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/libobject
            CMAKE_ARGS ${BUILD_EXTERNAL_ARGS})
    else ()
        message("git clone and build libobject")
        ExternalProject_Add(libobject
            SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/libobject
            GIT_REPOSITORY git@github.com:a1an1in/libobject.git
            CMAKE_ARGS ${BUILD_EXTERNAL_ARGS})
    endif()
endmacro()

macro (build_windows_dl)
    if ("${PLATFORM}" STREQUAL "windows")
        if(EXISTS ${PROJECT_SOURCE_DIR}/3rd/libobject)
            message("rebuild windows-dl")
            ExternalProject_Add(windows-dl
                SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/windows-dl
                CMAKE_ARGS ${BUILD_EXTERNAL_ARGS})
        else ()
            message("git clone and build windows-dl")
            ExternalProject_Add(windows-dl
                SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/windows-dl
                GIT_REPOSITORY https://github.com/a1an1in/windows-dl.git
                CMAKE_ARGS ${BUILD_EXTERNAL_ARGS})
        endif()
    endif()
endmacro()


# file(GLOB_RECURSE LIBOBJECT ${CMAKE_INSTALL_PREFIX}/lib/libobject.*)
# if ("${LIBOBJECT}" STREQUAL "")
#     build_libobject()
# else ()
#     MESSAGE("-- libobject path is ${LIBOBJECT}")
# endif()

build_windows_dl()