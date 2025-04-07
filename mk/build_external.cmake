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
    if(EXISTS ${PROJECT_SOURCE_DIR}/sysroot/windows/lib/libdl.dll.a)
        # return()
    endif()
    if ("${PLATFORM}" STREQUAL "windows")
        if(EXISTS ${PROJECT_SOURCE_DIR}/3rd/windows-dl)
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

macro (build_mysql)
    if ("${PLATFORM}" STREQUAL "linux")
        if(EXISTS ${PROJECT_SOURCE_DIR}/3rd/mysql-connector-c)
            message("rebuild mysql")
            ExternalProject_Add(mysql-connector-c
                SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/mysql-connector-c
                CMAKE_ARGS ${BUILD_EXTERNAL_ARGS})
        else ()
            message("git clone and build mysql")
            ExternalProject_Add(mysql-connector-c
                SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/mysql-connector-c
                GIT_REPOSITORY https://github.com/a1an1in/mysql-connector-c.git
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

macro (build_openssl)
    if ("${PLATFORM}" STREQUAL "linux")
        if(EXISTS ${PROJECT_SOURCE_DIR}/3rd/openssl)
            message("rebuild openssl")
            ExternalProject_Add(openssl 
                PREFIX openssl
                SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/openssl
                #BINARY_DIR ${PROJECT_SOURCE_DIR}/build/3rd
                CONFIGURE_COMMAND ./Configure --prefix=${CMAKE_INSTALL_PREFIX}
                BUILD_COMMAND make VERBOSE=1
                BUILD_IN_SOURCE TRUE)
        else ()
            message("git clone and build openssl")
            ExternalProject_Add(openssl
                PREFIX openssl
                SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rd/openssl
                GIT_REPOSITORY https://github.com/a1an1in/openssl.git
                CONFIGURE_COMMAND ./config --prefix=${CMAKE_INSTALL_PREFIX}
                BUILD_COMMAND make VERBOSE=1
                BUILD_IN_SOURCE TRUE)
        endif()
    endif()
endmacro()

#build_windows_dl()
#build_openssl()
#build_mysql()